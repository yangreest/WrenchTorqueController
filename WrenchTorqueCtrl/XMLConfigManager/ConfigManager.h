#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QDomDocument>
#include <QHash>
#include <QFile>
#include <QDebug>
#include "qcoreapplication.h"
#include "ConfigData.h"

/**
 * @brief 配置管理器类 - 单例模式
 * 负责XML配置文件的读取、解析和配置数据的管理
 */
class ConfigManager : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 获取ConfigManager单例实例
     * @return ConfigManager* 单例指针
     */
    static ConfigManager* getInstance();
    
    /**
     * @brief 加载配置文件
     * @param filePath 配置文件路径
     * @return bool 是否加载成功
     */
    bool loadConfig(const QString& filePath);
    
    /**
     * @brief 获取配置值
     * @param section 配置段名
     * @param key 配置键名
     * @param defaultValue 默认值
     * @return QVariant 配置值
     */
    //QVariant getValue(const QString& section, const QString& key, const QVariant& defaultValue = QVariant());
    
    /**
     * @brief 设置配置值
     * @param section 配置段名
     * @param key 配置键名
     * @param value 配置值
     */
    //void setValue(const QString& section, const QString& key, const QVariant& value);
    
    /**
     * @brief 保存配置到文件
     * @return bool 是否保存成功
     */
    bool saveConfig();
    
    /**
     * @brief 获取完整的配置数据
     * @return ConfigData 配置数据结构体
     */
    ConfigData getConfigData() const;

    MConfigDataMap getConfigDataMap() const;
    
private:
    /**
     * @brief 私有构造函数 - 单例模式
     * @param parent 父对象
     */
    ConfigManager(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ConfigManager();
    
    /**
     * @brief 解析配置文件内容到内存数据结构
     */
    void parseConfig();
    
    /**
     * @brief 从DOM元素获取子元素值
     * @param parentElement 父元素
     * @param tagName 子元素标签名
     * @param defaultValue 默认值
     * @return QString 子元素值
     */
    QString getElementValue(const QDomElement& parentElement, const QString& tagName, ConfigDataItem& cfgItem, const QString& defaultValue = "");

    /**
     * @brief 从元素中获取属性
     * @param Element 元素
     * @param tagName 子元素标签名
     * @brief 从元素中获取属性值
     * @param defaultValue 默认值
     * @return int 子元素值
     */

    
    static ConfigManager* m_instance;  // 单例实例指针

    QString m_filePath;                // 配置文件路径
    QDomDocument m_configDoc;          // XML文档对象
    MConfigDataMap m_configData;  // 配置数据哈希表
    ConfigData m_parsedConfigData;     // 解析后的配置数据结构体
};

#endif // CONFIGMANAGER_H