#include <QApplication>
#include <QMainWindow>
#include "thoMainWindow.hpp"
#include "thoInputHandler.hpp"
#include "thoSerialization.hpp"

using namespace std;
using thor::sm;

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    //Create and show the main window
    thor::MainWindow mw;
    mw.show();

    // Serialization Manager...
    sm::DeviceParams params= { "/dev/ttyUSB0" };
    sm::registerType<rat::StrCmdData>("StrCmdData", 1);
    sm::registerType<rat::BasicType<uint32_t, 6> >("BasicType2", 1);
    sm::registerType<rat::BasicType<int32_t, 3> >("BasicType3", 1);
    sm::registerType<rat::LISData>("LISData", 1);
    sm::registerType<rat::BasicType<float, 2> >("BasicType4", 1);

    sm::InputHandler handler(&mw);
    sm::startService(params, handler);

    return a.exec();
}
