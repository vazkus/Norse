#include "hreSerialization.hpp"
#include "ratSerializableTypes.hpp"
#include "broLISAccelerometer.hpp"

#include <stdio.h>
#include <math.h>

using namespace chibios_rt;

// adding missing symbols...
// TBD: move these to appropriate source-files...
void *__dso_handle = (void*) &__dso_handle;
void __cxa_atexit(void (*arg1)(void*), void* arg2, void* arg3)
{
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return;
}

class ChInputHandler
{
public:
    void process(ygg::TypeBase* d)
    {
        // main loop
        if(sm::isType<rat::StrCmdData>(d)) {
            rat::StrCmdData* sd = (rat::StrCmdData*)d;
            sm::send(new rat::StrCmdData(sd->string()));
        } else
        if(sm::isType<rat::PingData>(d)) {
            // bounce back the received ping packet
            rat::PingData* pd = (rat::PingData*)d;
            sm::send(new rat::PingData(pd->timeStamp()));
        } else
        if(sm::isType<rat::BasicType<float,2> >(d)) {
        } else {
        }
    }
};

int main(void) 
{
    // initializie ChibiOS
    halInit();
    System::Init();

    bro::LISAccelerometer accSensor;

    // register some useful and dummy types...
    sm::registerType<rat::BasicType<float, 2> >("BasicType4", 1);
    sm::registerType<rat::LISData>("LISData", 1);
    sm::registerType<rat::StrCmdData>("StrCmdData", 1);
    sm::registerType<rat::PingData>("PingData", 1);

    // instantiate the input-handler 
    sm::InputHandler handler;

    // setup the serial device
    sm::DeviceParams params = 
    {
        &SD2, 
        {115200, 0, USART_CR2_STOP1_BITS | USART_CR2_LINEN, 0},
        GPIOA, 3, PAL_MODE_ALTERNATE(7), // for RX
        GPIOA, 2, PAL_MODE_ALTERNATE(7)  // for TX
    };
    sm::Device device(params);
    if(!device.isOpen()) {
        return 1;
    }
    // setup the transport
    sm::Transport transport(&device);
    sm::Logger logger;
    // start the service
    sm::startService(transport, logger, handler);
    

    // Note that the serialization service is configured as NONBLOCKING
    // so we need while(true) trap so that the application won't quit...
    while (true) {
        chThdSleepMilliseconds(50);
        sm::send(new rat::LISData(accSensor.sample()));
    }

    return 0;
}
