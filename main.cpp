#include <QApplication>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

#include "database.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QApplication::setStyle("Fusion");

    QPalette pal;
    pal.setColor(QPalette::Window, QColor("#f4f5f7"));
    pal.setColor(QPalette::Base, QColor("#ffffff"));
    pal.setColor(QPalette::AlternateBase, QColor("#f0f2f4"));
    pal.setColor(QPalette::Text, QColor("#222222"));
    pal.setColor(QPalette::ButtonText, QColor("#222222"));
    pal.setColor(QPalette::WindowText, QColor("#222222"));
    pal.setColor(QPalette::Highlight, QColor("#145c3b"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    pal.setColor(QPalette::Button, QColor("#e3e7eb"));
    pal.setColor(QPalette::Midlight, QColor("#d0d6dd"));
    pal.setColor(QPalette::Shadow, QColor("#9da5b0"));
    a.setPalette(pal);

    QFile styleFile(":/qss/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        a.setStyleSheet(styleSheet);
    }

    const QString dbPath =
        QDir(QCoreApplication::applicationDirPath()).filePath("users.sqlite");

    if (!database::init(dbPath))
        return -1;

    MainWindow w;
    w.showMaximized();
    return a.exec();
}
