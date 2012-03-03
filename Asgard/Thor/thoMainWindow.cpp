#include <QtGui>
#include <sstream>
#include "thoMainWindow.hpp"
#include "thoSerialization.hpp"
#include "ratSerializableTypes.hpp"

namespace thor{

MainWindow::MainWindow()
{
    createActions();
    createMenus();
    createStatusBar();
    createDockWindows();
    setWindowTitle(tr("[Thor]"));
    setMinimumSize(640, 480);
    setUnifiedTitleAndToolBarOnMac(true);
}

void 
MainWindow::onAbout()
{
    QMessageBox::about(this, 
                       tr("About [Thor]"),
                       tr("blablabla"));
}

void 
MainWindow::createActions()
{
    mQuitAct = new QAction(tr("&Quit"), this);
    mQuitAct->setShortcuts(QKeySequence::Quit);
    mQuitAct->setStatusTip(tr("Quit the application"));
    connect(mQuitAct, SIGNAL(triggered()), this, SLOT(close()));

    mAboutAct = new QAction(tr("&About"), this);
    mAboutAct->setStatusTip(tr("Show the application's About box"));
    connect(mAboutAct, SIGNAL(triggered()), this, SLOT(onAbout()));
}

void 
MainWindow::createMenus()
{
    mViewMenu = menuBar()->addMenu(tr("&View"));

    menuBar()->addSeparator();

    mHelpMenu = menuBar()->addMenu(tr("&Help"));
    mHelpMenu->addAction(mAboutAct);
}

void 
MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void 
MainWindow::createDockWindows()
{
    QDockWidget *dock;

    // add the only widget - the log...
    mLogWidget = new QTextEdit;
    mLogWidget->setReadOnly(true);
    dock = new QDockWidget(tr("Log"), this);
    dock->setWidget(mLogWidget);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    mViewMenu->addAction(dock->toggleViewAction());
}

const QTextEdit* 
MainWindow::getLogWidget()
{
    return mLogWidget;
}

} // namespace thor
