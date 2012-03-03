#ifndef THO_MAIN_WINDOW_HPP
#define THO_MAIN_WINDOW_HPP

#include <QMainWindow>

class QAction;
class QListWidget;
class QMenu;
class QTextEdit;

namespace thor {

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

public:
    // widget accessor methods
    const QTextEdit* getLogWidget();

private slots:
    void onAbout();

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void createDockWindows();

private:
    QTextEdit*  mLogWidget;
    QMenu*   mViewMenu;
    QMenu*   mHelpMenu;
    QAction* mAboutAct;
    QAction* mQuitAct;
};

} // namespace thor

#endif // THO_MAIN_WINDOW_HPP
