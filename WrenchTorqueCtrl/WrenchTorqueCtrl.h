#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_WrenchTorqueCtrl.h"

class WrenchTorqueCtrl : public QMainWindow
{
    Q_OBJECT

public:
    WrenchTorqueCtrl(QWidget *parent = nullptr);
    ~WrenchTorqueCtrl();

private:
    Ui::WrenchTorqueCtrlClass ui;
};

