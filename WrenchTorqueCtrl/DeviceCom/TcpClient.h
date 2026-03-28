/*
 * @@File: 
 * @@version: v1.0
 * @@Author: Y.Zheng &SC
 * @Date: 2026-03-06 21:51:07
 * @LastEditTime: 2026-03-17 11:34:49
 * @LastEditors: Y.Zheng
 * @@Desc: 
 */
#pragma once
#include <condition_variable>
#include <queue>
#include <string>
#include <mutex>
#include <functional>

class CTcpClientCom
{
public:
	CTcpClientCom();
	~CTcpClientCom();  // 添加析构函数声明
	void SetParam(const char* pComName, int nComPort) ;
	void RegisterConnectStatusCallBack(const std::function<void(bool, int)>& f);
	bool BeginWork() ;
	bool EndWork() ;
	    // 同步发送数据并等待响应
    bool SyncWrite(const uint8_t* sendData, size_t sendLen, 
                   uint8_t* receiveBuffer, size_t bufferSize, 
                   size_t& receivedLen, int timeoutMs = 5000);
    
private:
	void ConnectionThread();
	void Connect();
	void CloseSocket();
	void SetConnected(bool b);

	    // 同步接收辅助函数
    bool WaitForSyncResponse(uint8_t* buffer_t, size_t bufferSize, 
                            size_t& receivedLen, int timeoutMs);

	std::string m_strTargetIp;

	uint16_t m_wTargetPort;

	uint64_t m_socket = 0;

	std::atomic<bool> m_running;
	std::atomic<bool> m_connected; // 

	std::thread m_connectionThread;

	std::function<void(bool, int)> m_connectCallback;
};
