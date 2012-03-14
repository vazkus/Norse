#ifndef YGG_REPLAY_MANAGER_HPP
#define YGG_REPLAY_MANAGER_HPP

#include "yggTransport.hpp"
#include "yggTransportImpl.hpp"
#include "yggSerializer.hpp"
#include "yggDeserializer.hpp"
#include <cstddef>

namespace ygg 
{

template <typename S, typename I, typename T, typename C>
class ReplayManager
{
public:
    typedef S SystemTraits;
    typedef I InputHandler;
    typedef T Terminator;
    typedef C Configuration;
    typedef typename S::MutexType  Mutex;
    typedef typename S::CondType   Condition;
    typedef typename S::ThreadType Thread;
    typedef typename S::DeviceType Device;
    typedef typename S::Utils      Utils;
    typedef ygg::DummySerializer   Serializer;
    typedef ConfiguredTransport<C,Device>      Transport;
    typedef ConfiguredTransport<C,DummyDevice> Logger;
    typedef typename Device::Params            DeviceParams;
    typedef ygg::Deserializer<S,Serializer,I,Logger,C> Deserializer;

public:
    // API used for the service initialization/start/stop.
    static void startReplay(Transport& transport, I& handler, T& terminator);
    static void stopReplay();
    static void pauseReplay();
    static void continueReplay();

private:
    ReplayManager();
    static ReplayManager<S,I,T,C>& self();

private:
    Deserializer* mDeserializer;
};

template <typename S, typename I, typename T, typename C>
ReplayManager<S,I,T,C>::ReplayManager()
{
}

template <typename S, typename I, typename T, typename C>
ReplayManager<S,I,T,C>& 
ReplayManager<S,I,T,C>::self()
{
    static ReplayManager<S,I,T,C> sRManager;
    return sRManager;
}

/////////////////////////////////////////////////////////
// function definition area...                         //
/////////////////////////////////////////////////////////
template <typename S, typename I, typename T, typename C>
void 
ReplayManager<S,I,T,C>::startReplay(Transport& transport, I& handler, T& terminator)
{
    TypeRegistry::initialize();
    // start the transport
    transport.start();
    if(transport.isError()) {
        return;
    }
    // create static instances
    static Serializer sSerializer(transport);
    // construct the deserializer
    self().mDeserializer = new Deserializer(transport, sSerializer, handler);
}


template <typename S, typename I, typename T, typename C>
void 
ReplayManager<S,I,T,C>::stopReplay()
{
}

template <typename S, typename I, typename T, typename C>
void 
ReplayManager<S,I,T,C>::pauseReplay()
{
}

template <typename S, typename I, typename T, typename C>
void 
ReplayManager<S,I,T,C>::continueReplay()
{
}

} // namespace ygg

#endif //YGG_REPLAY_MANAGER_HPP
