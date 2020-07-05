#include "sortwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SortWindow w;
    w.show();
    return a.exec();
}
