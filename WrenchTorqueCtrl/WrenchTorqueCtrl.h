#pragma once

#include <QtWidgets/QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QByteArray>
#include <QVBoxLayout>
#include <QLabel>
#include "ui_WrenchTorqueCtrl.h"
#include "DeviceCom/IDeviceCom.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <sstream>


// 解析后的数据结构体
struct ProtocolData {
    uint8_t main_func_code;       // 主功能码
    uint8_t sub_func_code;        // 附功能码
    uint32_t data_length;         // 传输数据长度
    std::vector<uint8_t> payload; // 传输数据（JSON/二进制）
    QString json_data;        // JSON数据
    uint8_t checksum;             // 校验位
};

class WrenchTorqueCtrl : public QMainWindow
{
    Q_OBJECT

public:

    // 协议相关常量定义
    const uint8_t FRAME_HEADER_CHAR1 = 0x54;  // 'T'
    const uint8_t FRAME_HEADER_CHAR2 = 0x57;  // 'W'
    const uint8_t FRAME_TAIL = 0xEF;          // 帧尾固定字节
    const size_t HEADER_FIXED_LENGTH = 8;     // 包头固定长度：固定字符(2)+主功能码(1)+附功能码(1)+数据长度(4)
    const size_t CHECKSUM_LENGTH = 1;         // 校验位长度（单字节）
    const size_t TAIL_LENGTH = 1;             // 帧尾长度

    

    WrenchTorqueCtrl(QWidget *parent = nullptr);
    ~WrenchTorqueCtrl();

    void InitUI();
    void InitParams();

    QTableWidgetItem* initCheckboxColumn();

    void ReceiveNewData(const uint8_t* p, int len);
    void ComDeviceConnectionChanged(const bool connected, int guid, int index);

    uint8_t calculate_checksum(const std::vector<uint8_t>& payload);
    ProtocolData parse_protocol_data(const std::vector<uint8_t>& raw_data);

    void add_json_data_to_table(QString json_data); // 添加JSON数据到表格

private slots:
    void on_actionSetting_triggered();
    void sendJsonData();

public:
    QNetworkAccessManager* networkManager;
    QWidget* m_Window;// ���ô���
    Ui::WrenchTorqueCtrlClass ui;

    IDeviceCom* m_pComDevice;

    std::vector<uint8_t> m_vectorDataBuffer;
};

