# Qt6.9 C++ XML配置解析与显示系统

## 项目概述

本项目实现了一个基于Qt6.9和C++的配置管理系统，能够从本地XML文件解析配置数据，并通过单例模式管理这些配置，最终在Qt界面上展示。

## 功能特点

1. **XML配置文件解析**：支持从本地XML文件读取配置数据
2. **单例模式管理**：使用单例模式管理配置数据，确保全局唯一访问点
3. **直观的界面展示**：
   - 左侧：一级参数列表（QListWidget）
   - 右上：二级参数表格（QTableWidget）
   - 右下：参数解释和用法说明（QTextEdit）
4. **配置数据结构**：包含DeviceControlBoard和Camera两个主要配置部分

## 技术栈

- Qt 6.9
- C++17
- Qt XML模块 (QtCore)
- Qt Widgets模块
- CMake构建系统

## 项目结构

```
ConfigParser/
├── CMakeLists.txt          # CMake构建文件
├── main.cpp                # 程序入口
├── ConfigManager.h         # 配置管理器头文件
├── ConfigManager.cpp       # 配置管理器实现
├── MainWindow.h            # 主窗口头文件
├── MainWindow.cpp          # 主窗口实现
├── ConfigData.h            # 配置数据结构定义
├── config.xml              # 示例配置文件
└── README.md               # 项目说明文档
```

## 配置文件格式

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Config>
    <DeviceControlBoard>
        <Ip>127.0.0.1</Ip>
        <Port>8234</Port>
        <DeviceHeartBeat>200</DeviceHeartBeat>
        <FactoryMode>0</FactoryMode>
        <UpAngle>120</UpAngle>
        <DownAngle>160</DownAngle>
        <WalkMotorSpeed>2</WalkMotorSpeed>
    </DeviceControlBoard>
    <Camera>
        <Left>192.168.1.11</Left>
        <Mid>192.168.1.10</Mid>
        <Right>192.168.1.10</Right>
    </Camera>
</Config>
```

## 构建与运行

### 前提条件

- Qt 6.9或更高版本
- CMake 3.16或更高版本
- 支持C++17的编译器

### 构建步骤

1. 克隆项目到本地
2. 创建构建目录
3. 运行CMake配置
4. 编译项目

```bash
# 示例构建命令
mkdir build
cd build
cmake ..
cmake --build .
```

### 运行程序

```bash
# 在构建目录中运行
cd bin
./ConfigParser
```

## 使用说明

1. 程序启动后会自动加载同目录下的`config.xml`文件
2. 在左侧列表中选择一级参数（DeviceControlBoard或Camera）
3. 右侧表格会显示对应的二级参数及其值
4. 右下区域会显示当前选中参数组的详细说明

## 代码说明

### 配置管理器（ConfigManager）

- 采用单例模式设计
- 负责XML文件的读取、解析和配置数据的管理
- 提供配置数据的获取和设置接口

### 主窗口（MainWindow）

- 实现了三区域布局的用户界面
- 处理用户交互和界面更新
- 显示配置参数和解释说明

### 配置数据结构（ConfigData）

- 定义了DeviceControlBoard和Camera两个配置结构体
- 用于存储解析后的配置数据

## 扩展与优化

1. **配置修改功能**：可以扩展实现配置参数的修改和保存
2. **错误处理**：增强错误处理和日志记录
3. **动态配置**：支持运行时重新加载配置文件
4. **多语言支持**：添加国际化支持
5. **更多配置类型**：扩展支持更多类型的配置数据

## 许可证

本项目采用MIT许可证。