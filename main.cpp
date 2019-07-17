#include "mainwindow.h"
#include <QApplication>


//我自己测试一个版本，看看能不能回退到指定的版本

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
