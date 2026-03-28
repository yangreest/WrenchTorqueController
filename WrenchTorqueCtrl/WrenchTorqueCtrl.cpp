#include "WrenchTorqueCtrl.h"
#include <QtConcurrent/QtConcurrent>
#include "XMLConfigManager/XmlManagerWindow.h"
#include <QInputDialog>

WrenchTorqueCtrl::WrenchTorqueCtrl(QWidget* parent)
	: QMainWindow(parent),
	history_data_number(0)
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
	ui.tableWidget->setColumnCount(6);

	// 2. 设置列标题
	QStringList columnHeaders;
	columnHeaders << "选择" << "序号" << "时间" << "扭矩" << "角度" << "Z轴";
	ui.tableWidget->setHorizontalHeaderLabels(columnHeaders);

	// 自动调整列宽（可选，适配内容）
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->resizeColumnsToContents();
	ui.tableWidget->setAlternatingRowColors(true); // 设置交替颜色
	// 列宽设置相等
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	// 绑定信号槽
	//connect(ui.actionSetting, &QAction::triggered, this, &WrenchTorqueCtrl::on_actionSetting_triggered);
	connect(ui.actionQuit, &QAction::triggered, this, &WrenchTorqueCtrl::close);
	connect(ui.sendBtn, &QPushButton::clicked, this, &WrenchTorqueCtrl::sendJsonData);
	//connect(ui.actionGetData, &QAction::triggered, this, &WrenchTorqueCtrl::on_actionGetData_triggered);
	connect(ui.actionSendData, &QAction::triggered, this, &WrenchTorqueCtrl::sendJsonData1);

	m_pTimer = new QTimer(this);
	connect(m_pTimer, &QTimer::timeout, this, &WrenchTorqueCtrl::on_timer_timeout);
	m_pTimer->start(10000);
}

void WrenchTorqueCtrl::InitParams()
{
	networkManager = new QNetworkAccessManager(this);

	m_pComDevice = new CTcpClientCom();

	m_pComDevice->SetParam(ConfigManager::getInstance()->getConfigData().wrench.ip.toStdString().c_str(), ConfigManager::getInstance()->getConfigData().wrench.port);
	//m_pComDevice->RegisterReadDataCallBack(std::bind(&WrenchTorqueCtrl::ReceiveNewData, this, std::placeholders::_1, std::placeholders::_2));
	m_pComDevice->RegisterConnectStatusCallBack(std::bind(&WrenchTorqueCtrl::ComDeviceConnectionChanged, this, std::placeholders::_1, std::placeholders::_2, 0));
	m_pComDevice->BeginWork();

}

// 初始化勾选框列
QTableWidgetItem* WrenchTorqueCtrl::initCheckboxColumn()
{
	// 创建勾选框单元格项
	QTableWidgetItem* checkboxItem = new QTableWidgetItem();
	// 设置为勾选框类型，默认未勾选
	checkboxItem->setCheckState(Qt::Unchecked);
	// 设置勾选框居中显示
	checkboxItem->setTextAlignment(Qt::AlignCenter);

	return checkboxItem;
}

void WrenchTorqueCtrl::ReceiveNewData(const uint8_t* p, int len)
{
	//1、先将数据添加至缓冲区最尾部
	auto oldSize = m_vectorDataBuffer.size();
	m_vectorDataBuffer.resize(oldSize + len);
	memcpy(m_vectorDataBuffer.data() + oldSize, p, len);
	try
	{
		ProtocolData result = parse_protocol_data(m_vectorDataBuffer);
		add_json_data_to_table(result.json_data);
	}
	catch (std::exception& e)
	{
		qWarning() << "解析数据失败：" << e.what();
		return;
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


void WrenchTorqueCtrl::on_actionSetting_triggered()
{
	if (m_Window == nullptr)
	{
		m_Window = new XmlManagerWindow();
	}
	m_Window->show();
}


void WrenchTorqueCtrl::on_actionGetData_triggered()
{
	//清空表格
	for (int i = ui.tableWidget->rowCount() - 1; i >= 0; i--)
	{
		ui.tableWidget->removeRow(i);
	}

	// 获取数据数量
	int number = GetHistoryDataNumber();
	history_data_number = number;

	readHistoryData(ConfigManager::getInstance()->getConfigData().wrench.lastItem);
}

int WrenchTorqueCtrl::GetHistoryDataNumber()
{
	// 准备发送数据
	uint8_t sendBuffer[] = { 0x54, 0x57, 0x0C , 0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xEF };  // 示例数据
	size_t sendLen = sizeof(sendBuffer);
	int data_count = 0;
	// 使用 QEventLoop 等待异步操作完成
	QEventLoop loop;
	bool operationCompleted = false;

	// 使用QtConcurrent异步执行
	QtConcurrent::run([this, sendBuffer, sendLen, &data_count, &loop, &operationCompleted]() {
		// 准备接收缓冲区
		uint8_t receiveBuffer[1024];
		size_t receivedLen = 0;

		// 测试同步写入
		bool success = m_pComDevice->SyncWrite(sendBuffer, sendLen,
			receiveBuffer, sizeof(receiveBuffer),
			receivedLen, 5000);  // 5秒超时

		// 在主线程中更新UI
		QMetaObject::invokeMethod(this, [this, success, receiveBuffer, receivedLen,&data_count, &loop, &operationCompleted]() {
			if (success) {
				qDebug() << "接收到数据：" << receivedLen << " 字节数据：";

				// receiveBuffer 到 m_vectorDataBuffer
				m_vectorDataBuffer.resize(receivedLen);
				memcpy(m_vectorDataBuffer.data(), receiveBuffer, receivedLen);

				ProtocolData result = parse_protocol_data(m_vectorDataBuffer);
				if (result.sub_func_code == 0x01)
				{
					data_count = add_json_data_to_counter(result.json_data);
				}
			}
			else {
				return 0;
				qDebug() << "同步发送失败或超时";
			}
			operationCompleted = true;
			loop.quit(); // 退出事件循环
			});
		});
	// 等待异步操作完成
	loop.exec();
	return data_count;
}

void WrenchTorqueCtrl::readHistoryData(int index)
{
	// 使用QtConcurrent异步执行
	// 使用 QEventLoop 等待异步操作完成
	QEventLoop loop;
	bool operationCompleted = false;
	QtConcurrent::run([this,index, &loop, &operationCompleted]() {
		for (int i = history_data_number - index; i <= history_data_number ; i++) {
			// 准备发送数据
			//uint8_t sendBuffer[] = { 0x54, 0x57, 0x0C , 0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xEF };  // 示例数据
			//size_t sendLen = sizeof(sendBuffer);

			// 创建一个JSON数据
			QJsonObject jsonObj;
			jsonObj.insert("history_data_index", i);
			//转成字符串
			QString jsonStr = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);

			std::vector< uint8_t> sendVector = packProtocolData(0x0C, 0x02, jsonStr.toStdString());

			uint8_t* sendBuffer = sendVector.data();
			size_t sendLen = sendVector.size();


			// 准备接收缓冲区
			uint8_t receiveBuffer[1024];
			size_t receivedLen = 0;

			// 测试同步写入
			bool success = m_pComDevice->SyncWrite(sendBuffer, sendLen,
				receiveBuffer, sizeof(receiveBuffer),
				receivedLen, 5000);  // 5秒超时

			// 在主线程中更新UI
			QMetaObject::invokeMethod(this, [this, success, receiveBuffer, receivedLen ,&loop, &operationCompleted]() {
				if (success) {
					qDebug() << "接收到数据：" << receivedLen << " 字节数据：";

					// receiveBuffer 到 m_vectorDataBuffer
					m_vectorDataBuffer.resize(receivedLen);
					memcpy(m_vectorDataBuffer.data(), receiveBuffer, receivedLen);

					ProtocolData result = parse_protocol_data(m_vectorDataBuffer);
					if (result.sub_func_code == 0x02)
					{
						add_json_data_to_table(result.json_data);
					}
				}
				else {
					qDebug() << "同步发送失败或超时";
				}
				});
		}
		operationCompleted = true;
		loop.quit(); // 退出事件循环
		});
    loop.exec();
}

void WrenchTorqueCtrl::sendLatestData()
{

}

void WrenchTorqueCtrl::on_actionRecordNext_triggered()
{
	// 弹出一个整数输入框
	QInputDialog inputDialog;
	inputDialog.setWindowTitle("输入数字");
	inputDialog.setLabelText("请输入数字：");
	inputDialog.setInputMode(QInputDialog::IntInput);
	inputDialog.setIntRange(1, 1000);

	if (inputDialog.exec() == QDialog::Accepted) {
		int number = inputDialog.intValue();
		qDebug() << "用户输入的数字是：" << number;

		// 创建一个JSON数据
		QJsonObject jsonObj;
		jsonObj.insert("history_data_index", number);
		//转成字符串
		QString jsonStr = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);

		std::vector< uint8_t> sendVector = packProtocolData(0x0C, 0x02, jsonStr.toStdString());

		uint8_t* sendBuffer = sendVector.data();
		size_t sendLen = sendVector.size();

		// 使用QtConcurrent异步执行
		QtConcurrent::run([=]() {
			// 准备接收缓冲区
			uint8_t receiveBuffer[1024];
			size_t receivedLen = 0;

			// 测试同步写入
			bool success = m_pComDevice->SyncWrite(sendBuffer, sendLen,
				receiveBuffer, sizeof(receiveBuffer),
				receivedLen, 5000);  // 5秒超时

			// 在主线程中更新UI
			QMetaObject::invokeMethod(this, [this, success, receiveBuffer, receivedLen]() {
				if (success) {
					qDebug() << "接收到数据：" << receivedLen << " 字节数据：";

					// receiveBuffer 到 m_vectorDataBuffer
					m_vectorDataBuffer.resize(receivedLen);
					memcpy(m_vectorDataBuffer.data(), receiveBuffer, receivedLen);

					ProtocolData result = parse_protocol_data(m_vectorDataBuffer);
					if (result.sub_func_code == 0x02)
					{
						add_json_data_to_table(result.json_data);
					}
				}
				else {
					qDebug() << "同步发送失败或超时";
				}
				});
			});
	}
}

// 发送JSON数据槽函数
void WrenchTorqueCtrl::sendJsonData1()
{
	ui.statusLabel->setText("正在发送请求...");

	// 1. 配置请求信息
	const QString targetUrl = ConfigManager::getInstance()->getConfigData().wrench.url;// "http://125.46.39.205:8088/wrench/tower/receiveData";
	QNetworkRequest request;
	request.setUrl(QUrl(targetUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=utf-8");
	request.setTransferTimeout(5000); // 设置传输超时为5秒
	// 构建单行数据的JSON
	QJsonObject jsonObj;
	int row = 0;
	// 获取各列的数据
	QTableWidgetItem* idItem = ui.tableWidget->item(row, 1); // 序号
	QTableWidgetItem* timeItem = ui.tableWidget->item(row, 2); // 时间
	QTableWidgetItem* torqueItem = ui.tableWidget->item(row, 3); // 扭矩
	QTableWidgetItem* angleItem = ui.tableWidget->item(row, 4); // 角度
	QTableWidgetItem* zAxisItem = ui.tableWidget->item(row, 5); // Z轴

	// 添加到JSON对象

	jsonObj.insert("torque", torqueItem ? torqueItem->text() : "");
	jsonObj.insert("angle", angleItem ? angleItem->text() : "");
	jsonObj.insert("z_axis", zAxisItem ? zAxisItem->text() : "");

	// 序列化JSON
	QJsonDocument jsonDoc(jsonObj);
	QByteArray postData = jsonDoc.toJson(QJsonDocument::Compact);
	qDebug() << "待发送的JSON数据：" << QString(postData);

	// 发送POST请求
	QNetworkReply* reply = networkManager->post(request, postData);

	// 绑定响应处理信号
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		
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
            ui.statusLabel->setText(QString("请求失败！错误信息：%1").arg(errorMsg));
		}
		// 释放资源
		reply->deleteLater();
		});
}
// 发送JSON数据槽函数 - 逐行发送版本
void WrenchTorqueCtrl::sendJsonData()
{
	ui.statusLabel->setText("正在发送请求...");

	// 获取表格中的所有有效行
	int rowCount = ui.tableWidget->rowCount();
	if (rowCount <= 0) {
		ui.statusLabel->setText("没有数据可发送");
		return;
	}

	// 存储选中的行索引
	QList<int> selectedRows;
	for (int row = 0; row < rowCount; row++) {
		QTableWidgetItem* checkboxItem = ui.tableWidget->item(row, 0);
		if (checkboxItem && checkboxItem->checkState() == Qt::Checked) {
			selectedRows.append(row);
		}
	}

	if (selectedRows.isEmpty()) {
		ui.statusLabel->setText("没有选中任何数据");
		return;
	}

	// 1. 配置请求信息
	const QString targetUrl = ConfigManager::getInstance()->getConfigData().wrench.url;
	QNetworkRequest request;
	request.setUrl(QUrl(targetUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=utf-8");
	request.setTransferTimeout(5000); // 设置传输超时为5秒

	// 逐行发送数据
	int totalToSend = selectedRows.size();
	int sentCount = 0;

	for (int row : selectedRows) {
		// 构建单行数据的JSON
		QJsonObject jsonObj;

		// 获取各列的数据
		QTableWidgetItem* idItem = ui.tableWidget->item(row, 1); // 序号
		QTableWidgetItem* timeItem = ui.tableWidget->item(row, 2); // 时间
		QTableWidgetItem* torqueItem = ui.tableWidget->item(row, 3); // 扭矩
		QTableWidgetItem* angleItem = ui.tableWidget->item(row, 4); // 角度
		QTableWidgetItem* zAxisItem = ui.tableWidget->item(row, 5); // Z轴

		// 添加到JSON对象

		jsonObj.insert("torque", torqueItem ? torqueItem->text() : "");
		jsonObj.insert("angle", angleItem ? angleItem->text() : "");
		jsonObj.insert("z_axis", zAxisItem ? zAxisItem->text() : "");

		// 序列化JSON
		QJsonDocument jsonDoc(jsonObj);
		QByteArray postData = jsonDoc.toJson(QJsonDocument::Compact);
		qDebug() << "待发送的JSON数据：" << QString(postData);

		// 发送POST请求
		QNetworkReply* reply = networkManager->post(request, postData);

		// 绑定响应处理信号
		connect(reply, &QNetworkReply::finished, this, [this, reply, &sentCount, totalToSend]() {
			sentCount++;
			if (reply->error() == QNetworkReply::NoError) {
				// 请求成功
				QByteArray responseData = reply->readAll();
				int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
				qDebug() << "请求成功！服务端响应：" << QString(responseData);
				qDebug() << "HTTP状态码：" << statusCode;

			}
			else {
				// 请求失败
				QString errorMsg = reply->errorString();
				int errorCode = (int)reply->error();
				int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
				qDebug() << "请求失败！错误信息：" << errorMsg;
				qDebug() << "错误代码：" << errorCode;
				qDebug() << "HTTP状态码：" << statusCode;
			}

			// 检查是否全部发送完毕
			if (sentCount >= totalToSend) {
				ui.statusLabel->setText(QString("全部数据发送完成，共发送 %1 条记录").arg(totalToSend));
			}

			// 释放资源
			reply->deleteLater();
			});
	}
}

void WrenchTorqueCtrl::on_timer_timeout()
{
	// 读取历史数据数量
	int History =  GetHistoryDataNumber();
	// 判断是否有更新
     if (History > history_data_number) {
		 history_data_number = History;
		// 读取最新的一个数据并保存到表格中
		 readHistoryData(0);
		 sendJsonData1();
	}
}

/**
 * @brief 计算传输数据的和校验（单字节，溢出循环从0开始）
 * @param payload 传输数据字节数组
 * @return 计算得到的校验位
 */
uint8_t WrenchTorqueCtrl::calculate_checksum(const std::vector<uint8_t>& payload) {
	uint32_t sum = 0;
	for (uint8_t byte : payload) {
		sum += byte;
	}
	// 单字节溢出循环（等价于取低8位）
	return static_cast<uint8_t>(sum & 0xFF);
}

/**
 * @brief 解析符合协议格式的来料数据
 * @param raw_data 原始字节流数据
 * @return 解析后的协议数据结构体
 * @throws std::invalid_argument 解析失败时抛出异常（长度不足、校验失败、帧头/帧尾错误等）
 */
ProtocolData WrenchTorqueCtrl::parse_protocol_data(const std::vector<uint8_t>& raw_data) {
	ProtocolData result;

	// 1. 检查原始数据最小长度：包头(8) + 至少1字节数据 + 校验位(1) + 帧尾(1)
	size_t min_required_length = HEADER_FIXED_LENGTH + 1 + CHECKSUM_LENGTH + TAIL_LENGTH;
	if (raw_data.size() < min_required_length) {
		throw std::invalid_argument("Raw data length is too short, minimum required: " + std::to_string(min_required_length));
	}

	// 2. 验证帧头（固定字符 'T'(0x54) + 'W'(0x57)）
	if (raw_data[0] != FRAME_HEADER_CHAR1 || raw_data[1] != FRAME_HEADER_CHAR2) {
		std::stringstream err_msg;
		err_msg << "Invalid frame header: 0x" << std::hex << static_cast<int>(raw_data[0])
			<< " 0x" << static_cast<int>(raw_data[1])
			<< ", expected: 0x54 0x57";
		throw std::invalid_argument(err_msg.str());
	}

	// 3. 解析包头部分
	result.main_func_code = raw_data[2];  // 主功能码（第3字节）
	result.sub_func_code = raw_data[3];   // 附功能码（第4字节）

	// 数据长度：4字节（第5-8字节），按小端模式解析
	result.data_length = static_cast<uint32_t>(raw_data[4]) |
		(static_cast<uint32_t>(raw_data[5]) << 8) |
		(static_cast<uint32_t>(raw_data[6]) << 16) |
		(static_cast<uint32_t>(raw_data[7]) << 24);

	// 4. 检查数据总长度是否匹配（包头+传输数据+校验位+帧尾）
	size_t expected_total_length = HEADER_FIXED_LENGTH + result.data_length + CHECKSUM_LENGTH + TAIL_LENGTH;
	if (raw_data.size() != expected_total_length) {
		std::stringstream err_msg;
		err_msg << "Data length mismatch: actual " << raw_data.size()
			<< ", expected " << expected_total_length
			<< " (header:8 + payload:" << result.data_length << " + checksum:1 + tail:1)";
		throw std::invalid_argument(err_msg.str());
	}

	// 5. 提取传输数据（payload）
	size_t payload_start = HEADER_FIXED_LENGTH;
	result.payload.assign(raw_data.begin() + payload_start,
		raw_data.begin() + payload_start + result.data_length);


	if (result.main_func_code == 0x0C)
	{
		if (result.sub_func_code == 0x02 || result.sub_func_code == 0x01)
		{
			// result.payload 转成json字符串
			result.json_data = QString::fromStdString(std::string(result.payload.begin(), result.payload.end()));
			qDebug() << "接收到的JSON数据：" << result.json_data;
		}
	}

	// 6. 提取并验证校验位
	size_t checksum_pos = payload_start + result.data_length;
	result.checksum = raw_data[checksum_pos];
	uint8_t calculated_checksum = calculate_checksum(result.payload);
	if (result.checksum != calculated_checksum) {
		std::stringstream err_msg;
		err_msg << "Checksum verification failed: received 0x" << std::hex << static_cast<int>(result.checksum)
			<< ", calculated 0x" << static_cast<int>(calculated_checksum);
		throw std::invalid_argument(err_msg.str());
	}

	// 7. 验证帧尾
	size_t tail_pos = checksum_pos + CHECKSUM_LENGTH;
	if (raw_data[tail_pos] != FRAME_TAIL) {
		std::stringstream err_msg;
		err_msg << "Invalid frame tail: 0x" << std::hex << static_cast<int>(raw_data[tail_pos])
			<< ", expected 0xEF";
		throw std::invalid_argument(err_msg.str());
	}

	return result;
}

std::vector<uint8_t> WrenchTorqueCtrl::packProtocolData(uint8_t mainFunc, uint8_t subFunc, const std::string& jsonStr)
{
	std::vector<uint8_t> sendBuffer;

	// 1. 包头部分（固定8字节）
	// 固定字符: 'T'(0x54) + 'W'(0x57)
	sendBuffer.push_back(FRAME_HEADER_CHAR1);
	sendBuffer.push_back(FRAME_HEADER_CHAR2);
	// 主功能码
	sendBuffer.push_back(mainFunc);
	// 附功能码
	sendBuffer.push_back(subFunc);

	// 数据长度（4字节，小端序）
	uint32_t dataLen = static_cast<uint32_t>(jsonStr.size());
	sendBuffer.push_back(dataLen & 0xFF);              // 最低字节（低8位）
	sendBuffer.push_back((dataLen >> 8) & 0xFF);       // 次低字节（8-15位）
	sendBuffer.push_back((dataLen >> 16) & 0xFF);      // 次高字节（16-23位）
	sendBuffer.push_back((dataLen >> 24) & 0xFF);      // 最高字节（24-31位）

	// 2. 传输数据部分（JSON字符串二进制数据）
	if (!jsonStr.empty()) {
		sendBuffer.insert(sendBuffer.end(), jsonStr.begin(), jsonStr.end());
	}

	// 3. 校验位（和校验，单字节）
	// 获取从第8位后的数据
	std::vector<uint8_t> payload(sendBuffer.begin() + HEADER_FIXED_LENGTH, sendBuffer.end());
	uint8_t checksum = calculate_checksum(payload);
	sendBuffer.push_back(checksum);

	// 4. 帧尾（固定字节0xEF）
	sendBuffer.push_back(FRAME_TAIL);

	return sendBuffer;
}

void WrenchTorqueCtrl::add_json_data_to_table(QString json_data)
{
	// 添加第一行
	ui.tableWidget->insertRow(0);
	// 解析JSON数据
	QJsonDocument json_doc = QJsonDocument::fromJson(json_data.toUtf8());
	QJsonObject json_obj = json_doc.object();
	int history_data_index = json_obj["history_data_index"].toInt();
	double torque = json_obj["torque"].toDouble();
	double angle = json_obj["angle"].toDouble();

	ui.tableWidget->setItem(0, 0, initCheckboxColumn());
	ui.tableWidget->setItem(0, 1, new QTableWidgetItem(QString::number(history_data_index)));
	ui.tableWidget->setItem(0, 3, new QTableWidgetItem(QString::number(torque)));
	ui.tableWidget->setItem(0, 4, new QTableWidgetItem(QString::number(angle)));
}

int WrenchTorqueCtrl::add_json_data_to_counter(QString json_data)
{
	QJsonDocument json_doc = QJsonDocument::fromJson(json_data.toUtf8());
	QJsonObject json_obj = json_doc.object();
	int h_d_n = json_obj["history_data_number"].toInt();
	qDebug() << "history_data_number:" << h_d_n;
	return h_d_n;

}
