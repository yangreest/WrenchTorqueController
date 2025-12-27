#include "XmlManagerWindow.h"
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QRegularExpression>
#include <QCheckBox>

XmlManagerWindow::XmlManagerWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	// 初始化UI
	initUI();

}

XmlManagerWindow::~XmlManagerWindow()
{
	// 清理资源
}

void XmlManagerWindow::initUI()
{
	try
	{
		MConfigDataMap mm = ConfigManager::getInstance()->getConfigDataMap();
		for (MConfigDataMap::iterator it = mm.begin(); it != mm.end(); ++it)
		{
			QListWidgetItem* item = new QListWidgetItem(it->first);
			ui.listWidget->addItem(item);
		}

		// 给ui.listWidget绑定槽函数
		connect(ui.listWidget, &QListWidget::currentItemChanged, this, &XmlManagerWindow::onParameterListCurrentItemChanged);
	}
	catch (std::exception& e)
	{
		QMessageBox::critical(this, "错误", e.what());
	}
}

void XmlManagerWindow::initTable(ConfigDataMap mm)
{
	// 表格分两列 ，第一列是参数名，第二列是参数值
	ui.tableWidget->setColumnCount(2);
	ui.tableWidget->setHorizontalHeaderLabels(QStringList() << "参数名" << "参数值");
	// table 拉伸策略
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui.tableWidget->setRowCount(mm.size());
	int row = 0;
	//将mm数据绘制到表格中
	for (ConfigDataMap::iterator it = mm.begin(); it != mm.end(); ++it)
	{
		QTableWidgetItem* item = new QTableWidgetItem(it->second.m_strName);
		// 将 ConfigDataItem 转成 QVariant 存在Item中
		item->setData(Qt::UserRole, QVariant::fromValue(it->second));
		ui.tableWidget->setItem(row, 0, item);

		// 根据 it->second.m_eDataType 判断第二列使用文本框还是下拉框
		switch (it->second.m_eDataType)
		{
		case ConfigDataItem::DataType::eBool:
		{
			QCheckBox* ckBox = new QCheckBox(ui.tableWidget);
			ckBox->setEnabled(true);
			ckBox->setChecked(it->second.m_strValue.toInt() == 0 ? true : false);
			ui.tableWidget->setCellWidget(row, 1, ckBox);
			break;
		}
		case ConfigDataItem::DataType::eInt:
		{
			QSpinBox* spinBox = new QSpinBox(ui.tableWidget);
			spinBox->setRange(it->second.m_strRange.split("-")[0].toInt(), it->second.m_strRange.split("-")[1].toInt());
			spinBox->setValue(it->second.m_strValue.toInt());
			// 2. 创建水平布局（控制水平对齐）+ 垂直居中（默认）
			//QHBoxLayout* layout = new QHBoxLayout(spinBox);
			//layout->setContentsMargins(2, 2, 2, 2); // 可选：留2px边距，避免控件贴单元格边框
			//layout->setSpacing(0); // 取消布局内间距

			//// 3. 设置布局对齐方式（核心）
			//layout->addWidget(spinBox);
			//layout->setAlignment(spinBox,Qt::AlignHCenter); // 给控件设置对齐
			ui.tableWidget->setCellWidget(row, 1, spinBox);
			break;
		}
		case ConfigDataItem::DataType::eString:
		{
			QLineEdit* lineEdit = new QLineEdit(ui.tableWidget);
			lineEdit->setText(it->second.m_strValue);
			ui.tableWidget->setCellWidget(row, 1, lineEdit);
			break;
		}
		case ConfigDataItem::DataType::eFloat:
		{
			QDoubleSpinBox* spinBox = new QDoubleSpinBox(ui.tableWidget);
			spinBox->setValue(it->second.m_strValue.toFloat());
			ui.tableWidget->setCellWidget(row, 1, spinBox);
			break;
		}
		case ConfigDataItem::DataType::eSelection:
		{
			QComboBox* comboBox = new QComboBox(ui.tableWidget);
			QStringList strList = it->second.m_strRange.split("-");
			comboBox->addItems(strList);
			comboBox->setCurrentIndex(it->second.m_strValue.toInt());
			ui.tableWidget->setCellWidget(row, 1, comboBox);
			break;
		}
		case ConfigDataItem::DataType::eIPAddress:
		{
			// 1. 设置输入校验器：仅允许数字和点
			QRegularExpression regExp("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){0,3}"
				"(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)?$");
			QRegularExpressionValidator* validator = new QRegularExpressionValidator(regExp, this);
			QLineEdit* lineEdit = new QLineEdit(ui.tableWidget);
			// lineEdit 的格式按照 IP 地址格式
			lineEdit->setValidator(validator);
			lineEdit->setText(it->second.m_strValue);
			ui.tableWidget->setCellWidget(row, 1, lineEdit);
			break;
		}
		default:
			break;
		}
		row++;
	}

}


void XmlManagerWindow::onParameterListCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	try
	{
		ConfigDataMap mm = ConfigManager::getInstance()->getConfigDataMap()[current->text()];
		initTable(mm);

	}
	catch (std::exception& e)
	{
		QMessageBox::critical(this, "错误", e.what());
	}
}