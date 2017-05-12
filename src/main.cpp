#include "MainGui.h"
#include <QApplication>

#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include "CreateDatabase.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon("Timer.ico"));
    if (!createDatabase()) return 1;

#if QT_VERSION < 0x050000
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

    MainGui mainGui;
    mainGui.show();

    return a.exec();
}
