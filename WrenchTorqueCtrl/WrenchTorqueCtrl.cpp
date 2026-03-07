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
	ui.tableWidget->setColumnCount(6);

	// 2. 设置列标题
	QStringList columnHeaders;
	columnHeaders << "选择" << "序号" << "时间" << "扭矩" << "角度" << "Z轴";
	ui.tableWidget->setHorizontalHeaderLabels(columnHeaders);

	// 自动调整列宽（可选，适配内容）
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->resizeColumnsToContents();

	// 绑定信号槽
	connect(ui.actionSetting, &QAction::triggered, this, &WrenchTorqueCtrl::on_actionSetting_triggered);
	connect(ui.sendBtn, &QPushButton::clicked, this, &WrenchTorqueCtrl::sendJsonData);
}

void WrenchTorqueCtrl::InitParams()
{
	networkManager = new QNetworkAccessManager(this);

	m_pComDevice = IDeviceCom::GetIDeviceCom(1);
	m_pComDevice->SetParam("127.0.0.1", 8234);
	m_pComDevice->RegisterReadDataCallBack(std::bind(&WrenchTorqueCtrl::ReceiveNewData, this, std::placeholders::_1, std::placeholders::_2));
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
		QMessageBox::warning(this, "错误", e.what());
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
		if (result.sub_func_code == 0x02)
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

void WrenchTorqueCtrl::add_json_data_to_table(QString json_data)
{
	ui.tableWidget->insertRow(ui.tableWidget->rowCount());
	// 解析JSON数据
	QJsonDocument json_doc = QJsonDocument::fromJson(json_data.toUtf8());
	QJsonObject json_obj = json_doc.object();
	int history_data_index = json_obj["history_data_index"].toInt();
	double torque = json_obj["torque"].toDouble();
	double angle = json_obj["angle"].toDouble();

	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 0, initCheckboxColumn());
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 1, new QTableWidgetItem(QString::number(history_data_index)));
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 3, new QTableWidgetItem(QString::number(torque)));
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 4, new QTableWidgetItem(QString::number(angle)));
}
