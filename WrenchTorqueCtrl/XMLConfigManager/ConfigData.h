#ifndef CONFIGDATA_H
#define CONFIGDATA_H

#include <QString>

/**
 * @brief DeviceControlBoard配置结构体
 */
struct DeviceControlBoardConfig
{
	QString ip;               // 设备IP地址
	int port;                 // 设备端口号
	int deviceHeartBeat;      // 心跳间隔(单位:ms)
	int factoryMode;          // 工厂模式标志
	int upAngle;              // 向上时的舵机角度(单位:ms)
	int downAngle;            // 向下时的舵机角度(单位:ms)
	int walkMotorSpeed;       // 行走电机速度(0:低速, 1:中速, 2:高速)
};

/**
 * @brief Camera配置结构体
 */
struct CameraConfig
{
	QString left;             // 左相机IP地址
	QString mid;              // 中间相机IP地址
	QString right;            // 右相机IP地址
};

/**
 * @brief 整体配置数据结构体
 */
struct ConfigData
{
	DeviceControlBoardConfig deviceControlBoard;  // 设备控制板配置
	CameraConfig camera;                          // 相机配置
};

class ConfigDataItem
{
public:
	enum  class DataType
	{
		eInt, // 整数
		eFloat, // 浮点
		eBool,	// 布尔
		eString, // 字符串
		eIPAddress,// ip地址
		eSelection, // 选择
	};

public:
	// 节点值
	QString m_strValue;
	// 节点参数值域的属性
	QString m_strRange;
	// 节点参数描述的属性
	QString m_strDescirption;
	// 节点显示参数名属性
	QString m_strName;
	// 节点参数类型
	DataType m_eDataType;
};

// 二级配置数据结构
typedef std::map<QString, ConfigDataItem> ConfigDataMap;

// 一级配置数据结构
typedef std::map<QString, ConfigDataMap> MConfigDataMap;
#endif // CONFIGDATA_H