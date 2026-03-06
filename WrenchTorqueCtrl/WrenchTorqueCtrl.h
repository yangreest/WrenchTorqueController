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

class WrenchTorqueCtrl : public QMainWindow
{
    Q_OBJECT

public:
    WrenchTorqueCtrl(QWidget *parent = nullptr);
    ~WrenchTorqueCtrl();

    void InitUI();
    void InitParams();

    void ReceiveNewData(const uint8_t* p, int len);
    void ComDeviceConnectionChanged(const bool connected, int guid, int index);
    bool Parse();

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

