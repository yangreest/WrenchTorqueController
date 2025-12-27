#include "WrenchTorqueCtrl.h"
<<<<<<< HEAD
#include "XMLConfigManager/XmlManagerWindow.h"

WrenchTorqueCtrl::WrenchTorqueCtrl(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

WrenchTorqueCtrl::~WrenchTorqueCtrl()
{
}

void WrenchTorqueCtrl::InitUI()
{

	// 绑定信号槽
	connect(ui.actionSetting, &QAction::triggered, this, &WrenchTorqueCtrl::on_actionSetting_triggered);
}

void WrenchTorqueCtrl::on_actionSetting_triggered()
{
	if (m_Window == nullptr)
	{
		m_Window = new XmlManagerWindow();
	}
	m_Window->show();
}
=======

WrenchTorqueCtrl::WrenchTorqueCtrl(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

WrenchTorqueCtrl::~WrenchTorqueCtrl()
{}
>>>>>>> a0a3917b59e2dbc465eba3b1aa03b7b96d732435

