#include "viewer.h"
#include <QtWidgets>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Viewer viewer;
    viewer.show();

    return app.exec();
};
