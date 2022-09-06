#include "viewer.h"
#include <QtWidgets>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("picoKVM");
    QCoreApplication::setApplicationName("picoKVMViewer");

    Viewer viewer;
    viewer.show();

    return app.exec();
};
