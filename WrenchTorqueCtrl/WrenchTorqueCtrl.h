#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_WrenchTorqueCtrl.h"

class WrenchTorqueCtrl : public QMainWindow
{
    Q_OBJECT

public:
    WrenchTorqueCtrl(QWidget *parent = nullptr);
    ~WrenchTorqueCtrl();

    void InitUI();

private slots:
    void on_actionSetting_triggered();

public:
    QWidget* m_Window;// ���ô���

private:
    Ui::WrenchTorqueCtrlClass ui;
};

