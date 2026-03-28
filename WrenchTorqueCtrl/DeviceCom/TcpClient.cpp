#include "TcpClient.h"

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <atomic>
#include <condition_variable>
#include <chrono>
// 在文件开头
#define NOMINMAX  // 如果使用Windows API
#include <algorithm>
#include <limits>

#ifndef _DEBUG
bool bWriteLog = true;
#include <iostream>
#include <sstream>
#include <iomanip>

#endif

#pragma comment(lib, "ws2_32.lib")

CTcpClientCom::CTcpClientCom() : m_wTargetPort(0)
{
	m_connectCallback = nullptr;
}

CTcpClientCom::~CTcpClientCom()
{
	m_running = false; // 确保线程退出
}

void CTcpClientCom::SetParam(const char* pComName, int nComPort)
{
	m_strTargetIp = pComName;
	m_wTargetPort = nComPort;
}

void CTcpClientCom::RegisterConnectStatusCallBack(const std::function<void(bool, int)>& f)
{
	m_connectCallback = f;
}

bool CTcpClientCom::BeginWork()
{
	if (m_running)
	{
		return true;
	}

	// 初始化Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}

	m_running = true;

	m_connected = false;

	// 启动连接线程
	m_connectionThread = std::thread(&CTcpClientCom::ConnectionThread, this);

	// 启动接收线程
	//m_receiveThread = std::thread(&CTcpClientCom::ReceiveThread, this);

	//启动发送线程
	//m_sendThread = std::thread(&CTcpClientCom::SendThread, this);

	return true;
}

bool CTcpClientCom::EndWork()
{
	if (!m_running)
	{
		return true;
	}

	m_running = false;

	

	// 关闭套接字
	CloseSocket();

	// 等待线程结束
	if (m_connectionThread.joinable())
	{
		m_connectionThread.join();
	}

	// 清理Winsock
	WSACleanup();
	return true;
}

void CTcpClientCom::ConnectionThread()
{
	while (m_running)
	{
		if (!m_connected)
		{
			Connect();
		}

		// 等待一段时间再尝试重连
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

// 设置连接状态
void CTcpClientCom::SetConnected(bool connected)
{
	if (m_connected != connected)
	{
		m_connected = connected;

		// 调用连接状态回调
		if (m_connectCallback != nullptr)
		{
			m_connectCallback(connected, 0);
		}
	}
}

void CTcpClientCom::CloseSocket()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	SetConnected(false);
}

void CTcpClientCom::Connect()
{
	CloseSocket();

	// 创建套接字
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	// 设置为非阻塞模式
	unsigned long nonBlocking = 1;
	if (ioctlsocket(m_socket, FIONBIO, &nonBlocking) != 0)
	{
		CloseSocket();
		return;
	}

	// 设置服务器地址
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(m_wTargetPort);

	// 转换IP地址
	inet_pton(AF_INET, m_strTargetIp.c_str(), &serverAddr.sin_addr);

	// 尝试连接
	int result = connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
		{
			// 连接正在进行中，使用select检查连接状态
			fd_set writeSet;
			timeval timeout;

			FD_ZERO(&writeSet);
			FD_SET(m_socket, &writeSet);
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

			result = select(0, nullptr, &writeSet, nullptr, &timeout);
			if (result > 0)
			{
				if (FD_ISSET(m_socket, &writeSet))
				{
					// 检查是否有错误
					int errorCode;
					int errorSize = sizeof(errorCode);
					if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorSize) == 0)
					{
						if (errorCode == 0)
						{
							SetConnected(true);
							return;
						}
					}
				}
			}
		}
	}
	else
	{
		SetConnected(true);
	}

	CloseSocket();
}

bool CTcpClientCom::SyncWrite(const uint8_t* sendData, size_t sendLen,
	uint8_t* receiveBuffer, size_t bufferSize,
	size_t& receivedLen, int timeoutMs)
{
	if (!m_connected || !sendData || !receiveBuffer || sendLen == 0)
		return false;

	std::cout << "SyncWrite \n" << std::endl;
	
	// 发送数据
	std::vector<uint8_t> dataVec(sendData, sendData + sendLen);
	{
		auto bytesSent = send(m_socket, reinterpret_cast<const char*>(dataVec.data()), dataVec.size(), 0);
#ifdef _DEBUG
		if (bWriteLog)
		{
			auto toHexString = [](const uint8_t* t, size_t  len)
				{
					std::ostringstream oss;
					oss << std::hex << std::setfill('0');
					for (int i = 0; i < len; ++i)
					{
						oss << std::setw(2) << static_cast<int>(t[i]);
						if (i < len - 1) oss << ' ';
					}
					return oss.str();
				};
			std::cout << "send data:" << toHexString(data.data(), data.size()) << std::endl;
		}
#endif
		if (bytesSent == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK && error != WSAEINTR)
			{
				SetConnected(false);
				return false;
			}
		}
	}

	// 等待响应
	bool result = WaitForSyncResponse(receiveBuffer, bufferSize, receivedLen, timeoutMs);

	return result;
}


bool CTcpClientCom::WaitForSyncResponse(uint8_t* buffer_t, size_t bufferSize,
	size_t& receivedLen, int timeoutMs)
{
	auto startTime = std::chrono::steady_clock::now();

	while (m_running && m_connected)
	{

		char buffer[4096];
		fd_set readSet;
		timeval timeout;

		FD_ZERO(&readSet);
		FD_SET(m_socket, &readSet);
		timeout.tv_sec = 1; // 1秒超时
		timeout.tv_usec = 0;

		// 使用select检查是否有数据可读
		int result = select(0, &readSet, nullptr, nullptr, &timeout);
		if (result > 0)
		{
			if (FD_ISSET(m_socket, &readSet))
			{
				int bytesReceived = recv(m_socket, buffer, sizeof(buffer) - 1, 0);
				if (bytesReceived > 0)
				{
#ifdef _DEBUG
					if (bWriteLog)
					{
						auto toHexString = [](const uint8_t* t, int len)
							{
								std::ostringstream oss;
								oss << std::hex << std::setfill('0');
								for (int i = 0; i < len; ++i)
								{
									oss << std::setw(2) << static_cast<int>(t[i]);
									if (i < len - 1) oss << ' ';
								}
								return oss.str();
							};
						std::cout << "get data:" << toHexString((uint8_t*)buffer, bytesReceived) << std::endl;
					}
#endif
                memcpy(buffer_t, buffer, bytesReceived);
                receivedLen = bytesReceived;

				return true;
				}
				else if (bytesReceived == 0)
				{
					// 连接关闭
					SetConnected(false);
				}
				else
				{
					// 接收错误
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK && error != WSAEINTR)
					{
						SetConnected(false);
					}
				}
			}
		}
		else if (result == 0)
		{
			// 检查是否超时
			auto currentTime = std::chrono::steady_clock::now();
			auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
				currentTime - startTime).count();

			if (elapsedMs >= timeoutMs)
				break;
		}
		else
		{
			// select错误
			int error = WSAGetLastError();
			SetConnected(false);
		}
	}
	return false;
}