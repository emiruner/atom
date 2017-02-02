#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    atom::aed::MainWindow mw;
    mw.show();

    return app.exec();
}
