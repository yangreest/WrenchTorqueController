#include "WrenchTorqueCtrl.h"
#include "XMLConfigManager/XmlManagerWindow.h"

WrenchTorqueCtrl::WrenchTorqueCtrl(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	InitUI();
	InitParams();
}

WrenchTorqueCtrl::~WrenchTorqueCtrl()
{
}

void WrenchTorqueCtrl::InitUI()
{
	// 创建表格
	
	// 绑定信号槽
	connect(ui.actionSetting, &QAction::triggered, this, &WrenchTorqueCtrl::on_actionSetting_triggered);
	connect(ui.sendBtn, &QPushButton::clicked, this, &WrenchTorqueCtrl::sendJsonData);
}

void WrenchTorqueCtrl::InitParams()
{
    networkManager = new QNetworkAccessManager(this);

	m_pComDevice = IDeviceCom::GetIDeviceCom(1);
	m_pComDevice->SetParam("127.0.0.1", 8234);
	m_pComDevice->RegisterReadDataCallBack(std::bind(&WrenchTorqueCtrl::ReceiveNewData,this, std::placeholders::_1,std::placeholders::_2));
	m_pComDevice->RegisterConnectStatusCallBack(std::bind(&WrenchTorqueCtrl::ComDeviceConnectionChanged, this,std::placeholders::_1, std::placeholders::_2, 0));
	m_pComDevice->BeginWork();

}

void WrenchTorqueCtrl::ReceiveNewData(const uint8_t* p, int len)
{
	//1、先将数据添加至缓冲区最尾部
	auto oldSize = m_vectorDataBuffer.size();
	m_vectorDataBuffer.resize(oldSize + len);
	memcpy(m_vectorDataBuffer.data() + oldSize, p, len);
	while (true)
	{
		if (!Parse())
		{
			break;
		}
	}
}

void WrenchTorqueCtrl::ComDeviceConnectionChanged(const bool connected, int guid, int index)
{
    if (connected)
    {
        ui.statusLabel->setText("已连接");
    }
    else
    {
        ui.statusLabel->setText("未连接");
    }
}

bool WrenchTorqueCtrl::Parse()
{
	while (m_vectorDataBuffer.size() > 8)
	{
		if (m_vectorDataBuffer[0] == 0xff && m_vectorDataBuffer[1] == 0xfe)
		{
			//找到了包头
			auto packLen = m_vectorDataBuffer[2];
			if (m_vectorDataBuffer.size() < packLen)
			{
				return false;
			}
			if (m_vectorDataBuffer[packLen - 2] == 0xfd && m_vectorDataBuffer[packLen - 1] == 0xfc)
			{
				//找到了包尾
				auto checkSum = m_vectorDataBuffer[packLen - 3];
				uint8_t checkSum2 = 0;
				for (int i = 0; i < packLen - 3; i++)
				{
					checkSum2 = checkSum2 + m_vectorDataBuffer[i];
				}
				//if (checkSum == checkSum2)
				//{
				//	m_cPackNumber = m_vectorDataBuffer[3];
				//	m_vectorCmdData.resize(packLen - 8);
				//	auto cmd = m_vectorDataBuffer[4];
				//	memcpy(m_vectorCmdData.data(), m_vectorDataBuffer.data() + 5, packLen - 8);
				//	Erase(packLen);
				//	if (m_function_Answer != nullptr)
				//	{
				//		Answer();
				//	}
				//	switch (cmd)
				//	{
				//	case 0:
				//	{
				//		DealHeartBeat();
				//		break;
				//	}
				//	case 0x07:
				//	{
				//		if (m_function_WriteLogCallBack != nullptr)
				//		{
				//			m_function_WriteLogCallBack(std::string(
				//				m_vectorCmdData.data(), m_vectorCmdData.data() + m_vectorCmdData.size()));
				//		}
				//		break;
				//	}
				//	case 0xa:
				//	{
				//		//进入OTA模式的反馈
				//		if (!m_vectorCmdData.empty())
				//		{
				//			if (m_vectorCmdData[0] > 0x00)
				//			{
				//				m_cOTAErrorCount = 0;
				//				m_cOTAStatus = 2;
				//				if (m_vectorCmdData[0] == 0x02)
				//				{
				//					m_bPauseHeartBeat = true;
				//				}
				//			}
				//		}
				//		break;
				//	}
				//	case 0x0b:
				//	{
				//		if (m_vectorCmdData.size() >= 5)
				//		{
				//			if (m_vectorCmdData[0] > 0x00)
				//			{
				//				m_cOTAErrorCount = 0;
				//				memcpy(&m_nOTAPackIndex, m_vectorCmdData.data() + 1, sizeof(uint32_t));
				//			}
				//		}
				//		break;
				//	}
				//	case 0x0c:
				//	{
				//		//进入OTA模式的反馈
				//		//if (!m_vectorCmdData.empty())
				//		//{
				//		//	if (m_vectorCmdData[0] > 0x00)
				//		//	{
				//		//		m_cOTAResult = 255;
				//		//	}
				//		//	else
				//		//	{
				//		//		m_cOTAResult = 254;
				//		//	}
				//		//}
				//		break;
				//	}
				//	case 0x11:
				//	{
				//		//传感器指令反馈
				//		if (m_vectorCmdData.size() >= 3)
				//		{
				//			switch (m_vectorCmdData[1])
				//			{
				//			case 3:
				//			case 2:
				//			case 4:
				//			{
				//				m_memCSensorData.m_cSensorIndex = m_vectorCmdData[0];
				//				m_memCSensorData.m_cCmd = m_vectorCmdData[1];
				//				break;
				//			}
				//			default:
				//			{
				//				break;
				//			}
				//			}

				//			memcpy(&(m_memCSensorData.m_wValue), m_vectorCmdData.data() + 2, sizeof(m_memCSensorData.m_wValue));
				//			if (m_function_SensorDataCallBack != nullptr)
				//			{
				//				m_function_SensorDataCallBack(&m_memCSensorData);
				//			}
				//		}
				//		break;
				//	}
				//	default:
				//	{
				//		break;
				//	}
				//	}
				//	return true;
				//}
				//
			}
			else
			{
				
			}
		}
		else
		{
			
		}
	}
	return false;
}

void WrenchTorqueCtrl::on_actionSetting_triggered()
{
	if (m_Window == nullptr)
	{
		m_Window = new XmlManagerWindow();
	}
	m_Window->show();
}


// 发送JSON数据槽函数
void WrenchTorqueCtrl::sendJsonData()
{
	ui.statusLabel->setText("正在发送请求...");

	// 1. 配置请求信息
	const QString targetUrl = "http://125.46.39.205:8088/wrench/tower/receiveData";
	QNetworkRequest request;
	request.setUrl(QUrl(targetUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=utf-8");
	request.setTransferTimeout(5000); // 设置传输超时为5秒
	//request.setAttribute(QNetworkRequest::TimeoutAttribute, 10000);

	// 2. 构建JSON数据
	QJsonObject jsonObj;
	jsonObj.insert("torque", "300.3");
	jsonObj.insert("angle", "");
	jsonObj.insert("gyro_z", "");
	// 可选：填充当前时间戳（格式可自定义）
	jsonObj.insert("timeStamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));

	// 3. 序列化JSON
	QJsonDocument jsonDoc(jsonObj);
	QByteArray postData = jsonDoc.toJson(QJsonDocument::Compact);
	qDebug() << "待发送的JSON数据：" << QString(postData);

	// 4. 异步发送POST请求（无阻塞，不影响界面响应）
	QNetworkReply* reply = networkManager->post(request, postData);

	// 5. 绑定响应处理信号（请求完成时触发）
	connect(reply, &QNetworkReply::finished, this, [=]() {
		if (reply->error() == QNetworkReply::NoError) {
			// 请求成功
			QByteArray responseData = reply->readAll();
			int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
			qDebug() << "请求成功！服务端响应：" << QString(responseData);
			qDebug() << "HTTP状态码：" << statusCode;
			ui.statusLabel->setText(QString("请求成功！HTTP状态码：%1").arg(statusCode));
		}
		else {
			// 请求失败
			QString errorMsg = reply->errorString();
			int errorCode = (int)reply->error();
			int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
			qDebug() << "请求失败！错误信息：" << errorMsg;
			qDebug() << "错误代码：" << errorCode;
			qDebug() << "HTTP状态码：" << statusCode;
			ui.statusLabel->setText(QString("请求失败！错误：%1").arg(errorMsg));
		}

		// 释放资源
		reply->deleteLater();
		});

}
