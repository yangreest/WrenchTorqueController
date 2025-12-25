#include "ConfigManager.h"

// 初始化静态成员变量
ConfigManager* ConfigManager::m_instance = nullptr;

ConfigManager* ConfigManager::getInstance()
{
	if (!m_instance)
	{
		m_instance = new ConfigManager();
	}
	return m_instance;
}

ConfigManager::ConfigManager(QObject* parent) : QObject(parent)
{
	// 初始化配置数据
	m_parsedConfigData.deviceControlBoard.ip = "127.0.0.1";
	m_parsedConfigData.deviceControlBoard.port = 8234;
	m_parsedConfigData.deviceControlBoard.deviceHeartBeat = 200;
	m_parsedConfigData.deviceControlBoard.factoryMode = 0;
	m_parsedConfigData.deviceControlBoard.upAngle = 120;
	m_parsedConfigData.deviceControlBoard.downAngle = 160;
	m_parsedConfigData.deviceControlBoard.walkMotorSpeed = 2;

	m_parsedConfigData.camera.left = "192.168.1.11";
	m_parsedConfigData.camera.mid = "192.168.1.10";
	m_parsedConfigData.camera.right = "192.168.1.10";

	//获取绝对路径
    QString appPath = QCoreApplication::applicationDirPath();
	QString configFilePath = appPath+ "\\config.xml";
	if (!loadConfig(configFilePath))
	{
		// 断言
        Q_ASSERT(false);
	}
}

ConfigManager::~ConfigManager()
{
	// 清理资源
}

bool ConfigManager::loadConfig(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Failed to open config file:" << filePath;
		return false;
	}

	if (!m_configDoc.setContent(&file))
	{
		file.close();
		qWarning() << "Failed to parse config file";
		return false;
	}

	file.close();
	m_filePath = filePath;

	// 解析XML内容到内存数据结构
	parseConfig();

	return true;
}

void ConfigManager::parseConfig()
{
	QDomElement root = m_configDoc.documentElement();
	if (root.tagName() != "Config")
	{
		qWarning() << "Invalid config file format: root tag is not 'Config'";
		return;
	}

	// 解析DeviceControlBoard配置
	QDomElement deviceControlBoardElement = root.firstChildElement("DeviceControlBoard");

	ConfigDataItem m_cfgItem;
	if (!deviceControlBoardElement.isNull())
	{
		m_cfgItem.m_strValue = getElementValue(deviceControlBoardElement, "Ip", m_cfgItem, "127.0.0.1");
		m_configData["DeviceControlBoard"]["Ip"] = m_cfgItem;
		m_parsedConfigData.deviceControlBoard.ip = m_cfgItem.m_strValue;

		m_cfgItem.m_strValue = getElementValue(deviceControlBoardElement, "Port", m_cfgItem, "8234");
		m_configData["DeviceControlBoard"]["Port"] = m_cfgItem;
		m_parsedConfigData.deviceControlBoard.port = m_cfgItem.m_strValue.toInt();

		m_cfgItem.m_strValue = getElementValue(deviceControlBoardElement, "DeviceHeartBeat", m_cfgItem, "200");
		m_configData["DeviceControlBoard"]["DeviceHeartBeat"] = m_cfgItem;
		m_parsedConfigData.deviceControlBoard.deviceHeartBeat = m_cfgItem.m_strValue.toInt();

		m_cfgItem.m_strValue = getElementValue(deviceControlBoardElement, "FactoryMode", m_cfgItem, "0");
        m_configData["DeviceControlBoard"]["FactoryMode"] = m_cfgItem;
        m_parsedConfigData.deviceControlBoard.factoryMode = m_cfgItem.m_strValue.toInt();

        m_cfgItem.m_strValue = getElementValue(deviceControlBoardElement, "UpAngle", m_cfgItem, "120");
        m_configData["DeviceControlBoard"]["UpAngle"] = m_cfgItem;
        m_parsedConfigData.deviceControlBoard.upAngle = m_cfgItem.m_strValue.toInt();

        m_cfgItem.m_strValue = getElementValue(deviceControlBoardElement, "DownAngle", m_cfgItem, "160");
        m_configData["DeviceControlBoard"]["DownAngle"] = m_cfgItem;
        m_parsedConfigData.deviceControlBoard.downAngle = m_cfgItem.m_strValue.toInt();

        m_cfgItem.m_strValue = getElementValue(deviceControlBoardElement, "WalkMotorSpeed", m_cfgItem, "2");
        m_configData["DeviceControlBoard"]["WalkMotorSpeed"] = m_cfgItem;
        m_parsedConfigData.deviceControlBoard.walkMotorSpeed = m_cfgItem.m_strValue.toInt();

	}

	// 解析Camera配置
	QDomElement cameraElement = root.firstChildElement("Camera");
	if (!cameraElement.isNull())
	{
        m_cfgItem.m_strValue = getElementValue(cameraElement, "Left", m_cfgItem, "192.168.1.11");
        m_configData["Camera"]["Left"] = m_cfgItem;
        m_parsedConfigData.camera.left = m_cfgItem.m_strValue;

        m_cfgItem.m_strValue = getElementValue(cameraElement, "Mid", m_cfgItem, "192.168.1.10");
        m_configData["Camera"]["Mid"] = m_cfgItem;
        m_parsedConfigData.camera.mid = m_cfgItem.m_strValue;

        m_cfgItem.m_strValue = getElementValue(cameraElement, "Right", m_cfgItem, "192.168.1.10");
        m_configData["Camera"]["Right"] = m_cfgItem;
        m_parsedConfigData.camera.right = m_cfgItem.m_strValue;
	}
}

QString ConfigManager::getElementValue(const QDomElement& parentElement, const QString& tagName, ConfigDataItem& cfgItem, const QString& defaultValue)
{
	QDomElement element = parentElement.firstChildElement(tagName);
	if (element.isNull())
	{
		return defaultValue;
	}
	cfgItem.m_strName = element.attribute("Name", "未知");
    cfgItem.m_strDescirption = element.attribute("Description", "未知");
    cfgItem.m_strRange = element.attribute("Range", "未知");
    cfgItem.m_eDataType = static_cast<ConfigDataItem::DataType> (element.attribute("DataType", "0").toInt());
	return element.text();
}

//QVariant ConfigManager::getValue(const QString& section, const QString& key, const QVariant& defaultValue)
//{
//    if (m_configData.contains(section) && m_configData[section].contains(key))
//    {
//        return m_configData[section][key];
//    }
//    return defaultValue;
//}
//
//void ConfigManager::setValue(const QString& section, const QString& key, const QVariant& value)
//{
//    m_configData[section][key] = value;
//    
//    // 更新对应的配置数据结构体
//    if (section == "DeviceControlBoard")
//    {
//        if (key == "Ip")
//            m_parsedConfigData.deviceControlBoard.ip = value.toString();
//        else if (key == "Port")
//            m_parsedConfigData.deviceControlBoard.port = value.toInt();
//        else if (key == "DeviceHeartBeat")
//            m_parsedConfigData.deviceControlBoard.deviceHeartBeat = value.toInt();
//        else if (key == "FactoryMode")
//            m_parsedConfigData.deviceControlBoard.factoryMode = value.toInt();
//        else if (key == "UpAngle")
//            m_parsedConfigData.deviceControlBoard.upAngle = value.toInt();
//        else if (key == "DownAngle")
//            m_parsedConfigData.deviceControlBoard.downAngle = value.toInt();
//        else if (key == "WalkMotorSpeed")
//            m_parsedConfigData.deviceControlBoard.walkMotorSpeed = value.toInt();
//    }
//    else if (section == "Camera")
//    {
//        if (key == "Left")
//            m_parsedConfigData.camera.left = value.toString();
//        else if (key == "Mid")
//            m_parsedConfigData.camera.mid = value.toString();
//        else if (key == "Right")
//            m_parsedConfigData.camera.right = value.toString();
//    }
//}

bool ConfigManager::saveConfig()
{
	if (m_filePath.isEmpty())
	{
		qWarning() << "No config file path specified";
		return false;
	}

	// 更新DOM文档
	QDomElement root = m_configDoc.documentElement();

	// 更新DeviceControlBoard配置
	QDomElement deviceControlBoardElement = root.firstChildElement("DeviceControlBoard");
	if (!deviceControlBoardElement.isNull())
	{
		QDomElement ipElement = deviceControlBoardElement.firstChildElement("Ip");
		if (!ipElement.isNull())
			ipElement.firstChild().setNodeValue(m_parsedConfigData.deviceControlBoard.ip);

		QDomElement portElement = deviceControlBoardElement.firstChildElement("Port");
		if (!portElement.isNull())
			portElement.firstChild().setNodeValue(QString::number(m_parsedConfigData.deviceControlBoard.port));

		// 更新其他元素...
	}

	// 更新Camera配置
	QDomElement cameraElement = root.firstChildElement("Camera");
	if (!cameraElement.isNull())
	{
		QDomElement leftElement = cameraElement.firstChildElement("Left");
		if (!leftElement.isNull())
			leftElement.firstChild().setNodeValue(m_parsedConfigData.camera.left);

		// 更新其他元素...
	}

	// 保存到文件
	QFile file(m_filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qWarning() << "Failed to open config file for writing:" << m_filePath;
		return false;
	}

	QTextStream out(&file);
	m_configDoc.save(out, 4);
	file.close();

	return true;
}

ConfigData ConfigManager::getConfigData() const
{
	return m_parsedConfigData;
}

MConfigDataMap ConfigManager::getConfigDataMap() const
{
	return m_configData;
}
