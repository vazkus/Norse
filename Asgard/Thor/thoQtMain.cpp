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
    sm::registerType<rat::StrCmdData>("StrCmdData", 1);
    sm::registerType<rat::BasicType<uint32_t, 6> >("BasicType2", 1);
    sm::registerType<rat::BasicType<int32_t, 3> >("BasicType3", 1);
    sm::registerType<rat::LISData>("LISData", 1);
    sm::registerType<rat::BasicType<float, 2> >("BasicType4", 1);

    // instantiate the input data handler type
    sm::InputHandler handler(&mw);
    // specifying uart device name and create the device...
    sm::DeviceParams params = { "/dev/ttyUSB0" };
    sm::Device device;
    // if successfull -> start the service...
    if(device.initialize(params)) {
        // start the service
        sm::startService(&device, &handler);
    }

    return a.exec();
}
