#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QByteArray>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>

class HttpJsonSenderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HttpJsonSenderWidget(QWidget* parent = nullptr) : QWidget(parent) {
        // 初始化界面
        QVBoxLayout* layout = new QVBoxLayout(this);
        QPushButton* sendBtn = new QPushButton("发送JSON数据", this);
        statusLabel = new QLabel("未发送请求", this);
        layout->addWidget(sendBtn);
        layout->addWidget(statusLabel);
        setLayout(layout);

        // 初始化网络管理器（类成员变量，避免重复创建）
        networkManager = new QNetworkAccessManager(this);

        // 绑定按钮点击信号
        connect(sendBtn, &QPushButton::clicked, this, &HttpJsonSenderWidget::sendJsonData);
    }

private slots:
    // 发送JSON数据槽函数
    void sendJsonData() {
        statusLabel->setText("正在发送请求...");

        // 1. 配置请求信息
        const QString targetUrl = "http://192.168.1.8:8088/wrench/tower/receiveData";
        QNetworkRequest request;
        request.setUrl(QUrl(targetUrl));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=utf-8");
        request.setTransferTimeout(5000); // 设置传输超时为5秒
        //request.setAttribute(QNetworkRequest::TimeoutAttribute, 10000);

        // 2. 构建JSON数据
        QJsonObject jsonObj;
        jsonObj.insert("torque", "300.3");
        jsonObj.insert("angle", "");
        jsonObj.insert("gyro_z", "");
        // 可选：填充当前时间戳（格式可自定义）
        jsonObj.insert("timeStamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));

        // 3. 序列化JSON
        QJsonDocument jsonDoc(jsonObj);
        QByteArray postData = jsonDoc.toJson(QJsonDocument::Compact);
        qDebug() << "待发送的JSON数据：" << QString(postData);

        // 4. 异步发送POST请求（无阻塞，不影响界面响应）
        QNetworkReply* reply = networkManager->post(request, postData);

        // 5. 绑定响应处理信号（请求完成时触发）
        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
                // 请求成功
                QByteArray responseData = reply->readAll();
                int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                qDebug() << "请求成功！服务端响应：" << QString(responseData);
                qDebug() << "HTTP状态码：" << statusCode;
                statusLabel->setText(QString("请求成功！HTTP状态码：%1").arg(statusCode));
            }
            else {
                // 请求失败
                QString errorMsg = reply->errorString();
                int errorCode = (int)reply->error();
                int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                qDebug() << "请求失败！错误信息：" << errorMsg;
                qDebug() << "错误代码：" << errorCode;
                qDebug() << "HTTP状态码：" << statusCode;
                statusLabel->setText(QString("请求失败！错误：%1").arg(errorMsg));
            }

            // 释放资源
            reply->deleteLater();
            });
    }

private:
    QNetworkAccessManager* networkManager;
    QLabel* statusLabel;
};

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    HttpJsonSenderWidget w;
    w.setWindowTitle("Qt6 HTTP JSON 发送工具");
    w.resize(400, 200);
    w.show();

    return a.exec();
}

#include "main.moc"  // GUI程序使用Q_OBJECT必须添加此句（qmake会自动生成moc文件）