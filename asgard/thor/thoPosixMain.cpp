#include "yggSerializationManager.hpp"
#include "yggReplayManager.hpp"
#include "yggPosixTraits.hpp"
#include "ratSerializableTypes.hpp"
#include <iostream>
#include <string>

using namespace std;
struct ThorPosixConfig
{
    // configuration of the serialization system
    const static ygg::ConfigCommunication   Serialization    = ygg::COMMUNICATION_NONBLOCKING;
    const static ygg::ConfigCommunication   Deserialization  = ygg::COMMUNICATION_NONBLOCKING;
    const static ygg::ConfigEndianness      Endianness       = ygg::ENDIAN_NATIVE;
    const static ygg::ConfigManifest        ManifestRequired = ygg::MANIFEST_REQUIRED;
    // various parameters of the serialization system
    const static int BasePriority = 0;
    const static int InputQueueSize = 10;
    const static int OutputQueueSize = 10;
    const static int ManifestRequestMs = 1000;
};
class PCInputHandler;

class PCTerminator
{
};

typedef ygg::SerializationManager<
                     ygg::PosixSystemTraits, 
                     PCInputHandler, 
                     ygg::ConfiguredTransport<ThorPosixConfig>, 
                     ThorPosixConfig
                    > sm;
typedef ygg::ReplayManager<
                     ygg::PosixSystemTraits, 
                     PCInputHandler, 
                     PCTerminator,
                     ThorPosixConfig
                    > rm;

class PCInputHandler
{
public:
    void process(ygg::TypeBase* d)
    {
        if(sm::isType<rat::StrCmdData>(d)) {
            rat::StrCmdData* sd = (rat::StrCmdData*)d;
            cout<<"IN: received string: "<<sd->string()<<endl;
        } else
        if(sm::isType<rat::PingData>(d)) {
            rat::PingData* pd = (rat::PingData*)d;
            std::cout<<"roundtrip: "<< sm::Utils::getMilliseconds() - pd->timeStamp()<<"ms"<<std::endl;
        } else 
        if(sm::isType<rat::LISData>(d)) {
            rat::LISData* ld = (rat::LISData*)d;
            rat::Axes a = ld->axes();
            std::cout<<"lis: ["<<(uint32_t)a.x<<", "<<(uint32_t)a.y<<", "<<(uint32_t)a.z<<"]"<<std::endl;
        } else {
            std::cout<<"none"<<std::endl;
        }
    }
};

static bool pingerFunc(void* param)
{
    sm::send(new rat::PingData(sm::Utils::getMilliseconds()));
    sm::Thread::sleepMilliseconds(200);
    std::cout<<"sending ping..."<<std::endl;
    return false;
}

int main()
{
    // register a dummy type
    sm::registerType<rat::BasicType<float, 2> >("BasicType4", 1);
    // register ping type
    sm::registerType<rat::PingData>("PingData", 1);
    // register special type that holds accelerometer readings...
    sm::registerType<rat::LISData>("LISData", 1);

    // instantiate the input data handler type
    PCInputHandler handler;

    // specifying uart device name and create the device...
    sm::DeviceParams params= { "/dev/ttyUSB0" };
    sm::Device device(params);
    if(!device.isOpen()) {
        return 1;
    }
    // setup the transport
    sm::Transport transport(&device);


    // now initialize the log device... 
    sm::DeviceParams lparams= { "logfile.out" };
    sm::Device ldevice(lparams);
    if(!ldevice.isOpen()) {
        return 1;
    }

    sm::Logger logger(&ldevice);

    // start the service
    sm::startService(transport, handler);
    sm::startLogger(logger);

    // add a thread that sends ping once in a while (set to ~5hz)
    sm::Thread pinger("Pinger", 0, 0, pingerFunc, NULL, NULL);
    // Note that the current configuration is non-blocking and we 
    // need the while(true) trap below...
    while(true) {
        sleep(1);
    }
    return 0;



    PCTerminator terminator;
    rm::startReplay(transport, handler, terminator);
}
