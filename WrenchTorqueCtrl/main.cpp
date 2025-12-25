#include "WrenchTorqueCtrl.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    WrenchTorqueCtrl window;
    window.show();
    return app.exec();
}
