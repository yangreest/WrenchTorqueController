#include <QApplication>
#include "HttpJsonSenderWidget.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    HttpJsonSenderWidget w;
    w.setWindowTitle("Qt6 HTTP JSON 发送工具");
    w.resize(400, 200);
    w.show();

    return a.exec();
}