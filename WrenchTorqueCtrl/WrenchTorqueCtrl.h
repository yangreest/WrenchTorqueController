#pragma once
<<<<<<< HEAD
=======

>>>>>>> a0a3917b59e2dbc465eba3b1aa03b7b96d732435
#include <QtWidgets/QMainWindow>
#include "ui_WrenchTorqueCtrl.h"

class WrenchTorqueCtrl : public QMainWindow
{
    Q_OBJECT

public:
    WrenchTorqueCtrl(QWidget *parent = nullptr);
    ~WrenchTorqueCtrl();

<<<<<<< HEAD
    void InitUI();

private slots:
    void on_actionSetting_triggered();

public:
    QWidget* m_Window;// 设置窗口

=======
>>>>>>> a0a3917b59e2dbc465eba3b1aa03b7b96d732435
private:
    Ui::WrenchTorqueCtrlClass ui;
};

