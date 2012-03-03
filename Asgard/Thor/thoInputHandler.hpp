#ifndef THO_INPUT_HANDLER_HPP
#define THO_INPUT_HANDLER_HPP

#include "thoSerialization.hpp"
#include "ratSerializableTypes.hpp"
#include <QObject>

namespace thor {

class MainWindow;

class InputHandler : public QObject
{
    Q_OBJECT
public:
    InputHandler(MainWindow* mainWin);
    ~InputHandler();
    void process(ygg::TypeBase* d);
signals:
    void logReceived(const QString& text);
private:
    MainWindow* mMain;
};

} // namespace thor 

#endif //THO_INPUT_HANDLER_HPP
