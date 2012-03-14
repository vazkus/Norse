#ifndef YGG_CHIBIOS_TRAITS_HPP
#define YGG_CHIBIOS_TRAITS_HPP

#include "ch.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

namespace ygg 
{

class ChibiosMutex : public chibios_rt::Mutex
{
public:
    void lock() 
    {
        Lock();
    }
    void unlock() 
    {
        Unlock();
    }
};

class ChibiosCondVar : public chibios_rt::CondVar
{
public:
    ChibiosCondVar(chibios_rt::Mutex& mutex) 
    {
        (void)mutex;
    }
    void wait() 
    {
        Wait();
    }
    void signal() 
    {
        Signal();
    }
};

class ChibiosThread 
{
public:
    typedef bool(*ThreadFunc)(void*);
    typedef bool(*FinalizeFunc)();
public:
    ChibiosThread(const char *name, uint32_t stack, tprio_t prio, 
                  ThreadFunc threadFunc, FinalizeFunc finalizeFunc = NULL, 
                  void* param = NULL)
     : mThreadRef(NULL),
       mName(name),
       mThreadFunc(threadFunc),
       mFinalizeFunc(finalizeFunc),
       mParam(param)
    {
        mThreadRef = chThdCreateFromHeap(NULL, stack, prio, thdStart, this);
    } 
    static msg_t thdStart(void *arg) 
    {

        return ((ChibiosThread*)arg)->run();
    }
    static void sleepMilliseconds(uint32_t ms)
    {
        chThdSleepMilliseconds(ms);
    }
protected:
    msg_t run(void)
    {
        bool shouldQuit = false;
        while(!shouldQuit) {
            shouldQuit = mThreadFunc(mParam);
        }
        if(mFinalizeFunc) {
            mFinalizeFunc();
        }
        return 0;
    }
private:
    ::Thread    *mThreadRef;
    const std::string& mName;
    ThreadFunc   mThreadFunc;
    FinalizeFunc mFinalizeFunc;
    void*        mParam;
};

class ChibiosDevice
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
        SerialDriver* mSD;
        SerialConfig  mConfig;
        ioportid_t    mRXPort;
        ioportmask_t  mRXMask; 
        iomode_t      mRXMode;
        ioportid_t    mTXPort;
        ioportmask_t  mTXMask;
        iomode_t      mTXMode;
    };

public:
    ChibiosDevice(const Params& params, const Mode)
    {
        mSD = params.mSD;
        assert(mSD);
        sdStart(mSD, &params.mConfig);
        palSetPadMode(params.mRXPort, 
                      params.mRXMask, 
                      params.mRXMode);
        palSetPadMode(params.mTXPort, 
                      params.mTXMask, 
                      params.mTXMode);
    }
    void close()
    {
        sdStop(mSD);
    }
    bool read(void* ptr, uint32_t size)
    {
        return sdRead(mSD, (uint8_t*)ptr, size) == size;
    }
    bool write(const void* ptr, uint32_t size) 
    {
        return sdWrite(mSD, (uint8_t*)ptr, size) == size;
    }
    bool isOpen() 
    {
        return mSD->state == SD_READY;
    }

private:
    SerialDriver* mSD;
};

class ChibiosUtils
{
public:
    static uint32_t getMilliseconds()
    {
        return ((chTimeNow()-1L)*1000L)/CH_FREQUENCY + 1L;
    }
};


class ChibiosSystemTraits
{
public:
    typedef ChibiosMutex       MutexType;
    typedef ChibiosCondVar     CondType;
    typedef ChibiosThread      ThreadType;
    typedef ChibiosDevice      DeviceType;
    typedef ChibiosUtils       Utils;
};

} //namespace ygg 

#endif //YGG_CHIBIOS_TRAITS_HPP
