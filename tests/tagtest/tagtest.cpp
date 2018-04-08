
#include "tagscanner.h"
#include "log.h"

#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QtGui/QApplication>
#endif
#include <QApplication>

int dummy;

    
void dummyfunc()
{
    dummy++;

}

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    TagScanner scanner;

    Q_UNUSED(argc);
    Q_UNUSED(argv);

    scanner.init();

    QList<Tag> taglist;
    if(scanner.scan("tagtest.cpp", &taglist))
        errorMsg("Failed to scan"); 

    scanner.dump(taglist);

    return 0;
}


