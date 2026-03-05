#include <QApplication>
#include "database.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!database::init("users.sqlite"))
        return -1;

    MainWindow w;
    w.show();
    return a.exec();
}
