#include <QApplication>
#include "mainwindow.h"
#include "database.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!database::init("users.sqlite"))
        return -1;

    MainWindow w;
    w.show();
    return a.exec();
}
