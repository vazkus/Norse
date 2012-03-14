#ifndef YGG_QT_TRAITS_HPP
#define YGG_QT_TRAITS_HPP

#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <QDateTime>
#include <QFile>
#include <stdint.h>
#include <sys/time.h>

namespace ygg
{

class QtCondVar : public QWaitCondition
{
public:
    QtCondVar(QMutex& mutex) 
     : mCondMutex(mutex) 
    {
    }
    void wait() 
    {
        QWaitCondition::wait(&mCondMutex);
    }
    void signal() 
    {
        wakeAll();
    }
    QMutex& mCondMutex;
};

class QtThread : public QThread
{
public:
    typedef bool(*ThreadFunc)(void*);
    typedef bool(*FinalizeFunc)();
public:
    QtThread(const char *tName, uint32_t size, uint32_t prio, 
             ThreadFunc threadFunc, FinalizeFunc finalizeFunc = NULL, 
             void* params = NULL)
     : mName(tName),
       mThreadFunc(threadFunc),
       mFinalizeFunc(finalizeFunc),
       mParams(params)
    {
        (void)size;
        (void)prio;
        start();
    }
    void run() 
    {
        bool should_quit = false;
        while(!should_quit) {       
            should_quit = mThreadFunc(mParams);
        }
        
        if(mFinalizeFunc) {
            mFinalizeFunc();
        }
    }
    static void sleepMilliseconds(uint32_t ms)
    {
        msleep(ms);
    }
private:
    std::string  mName;
    ThreadFunc   mThreadFunc;
    FinalizeFunc mFinalizeFunc;
    void*        mParams;
};

class QtDevice : public QFile
{
public:
    enum Mode 
    {
        IN,
        OUT,
        INOUT        
    };
    struct Params
    {
        QString mDeviceName;
    };
public:
    QtDevice(const Params& params, const Mode mode)
    {
        OpenModeFlag omode = QIODevice::NotOpen;
        switch(mode) {
            case IN:    omode = QIODevice::ReadOnly;
                        break;
            case OUT:   omode = QIODevice::WriteOnly;
                        break;
            case INOUT: omode = QIODevice::ReadWrite;
                        break;
        }
        setFileName(params.mDeviceName);
        open(omode);
    }
    void close()
    {
        // to be implemented
    }
    bool read(void* b, uint32_t size)
    {
        return QFile::readData((char*)b, size) == size;
    }
    bool write(const void* b, uint32_t size) 
    {
        return QFile::writeData((const char*)b, size) == size;
    }
    bool isOpen() 
    {
        return QFile::isOpen();
    }

};

class QtUtils
{
public:
    static uint32_t getMilliseconds()
    {
        return (uint32_t) QDateTime::currentMSecsSinceEpoch();
    }
};

class QtSystemTraits
{
public:
    typedef QMutex    MutexType;
    typedef QtCondVar CondType;
    typedef QtThread  ThreadType;
    typedef QtDevice  DeviceType;
    typedef QtUtils   Utils;
};

} // namespace ygg

#endif //YGG_QT_TRAITS_HPP
