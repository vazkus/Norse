#include "thoInputHandler.hpp"
#include "thoMainWindow.hpp"
#include <QTextEdit>
#include <iostream>
#include <sstream>

namespace thor {


InputHandler::InputHandler(MainWindow* mainWin)
  : QObject(NULL),
    mMain(mainWin)
{
    connect(this, SIGNAL(logReceived(const QString&)), 
            mMain->getLogWidget(), SLOT(append(const QString&)));
}

InputHandler::~InputHandler()
{
}

void
InputHandler::process(ygg::TypeBase* d)
{
    std::stringstream ss;
    if(sm::isType<rat::StrCmdData>(d)) {
        rat::StrCmdData* sd = (rat::StrCmdData*)d;
        ss<<"IN: "<<sd->string();
        emit logReceived(QString(ss.str().c_str()));
        ss.str("");
    } else
    if(sm::isType<rat::BasicType<float,2> >(d)) {
    } else 
    if(sm::isType<rat::LISData>(d)) {
        rat::LISData* ld = (rat::LISData*)d;
        rat::Axes a = ld->axes();
        ss<<"lis: ["<<(uint32_t)a.x<<", "<<(uint32_t)a.y<<", "<<(uint32_t)a.z<<"]";
        emit logReceived(QString(ss.str().c_str()));
        ss.str("");
    } else {
        std::cout<<"none"<<std::endl;
    }
}

} // namespace thor
