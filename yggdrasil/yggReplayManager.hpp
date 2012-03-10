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
    typedef ConfiguredTransport<C> Transport;
    typedef typename S::MutexType  Mutex;
    typedef typename S::CondType   Condition;
    typedef typename S::ThreadType Thread;
    typedef typename S::Utils      Utils;
    typedef ygg::DummyTransport    Logger;
    typedef ygg::DummySerializer   Serializer;
    typedef typename S::DeviceParamsType DeviceParams;
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
    // API for type registration and checking
    template<typename Type> static bool registerType(const std::string& name, 
                                                     const int version);
    template<typename Type> static bool isType(TypeBase* d);

private:
    TypeRegistry  mTypeRegistry;
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
    // start the transport
    transport.start();
    if(transport.isError()) {
        return;
    }
    transport.setTypeRegistry(&self().mTypeRegistry);
    
    // create static instances
    static Serializer sSerializer(transport);
    
    // construct the deserializer
    self().mDeserializer = new Deserializer(transport, self().mTypeRegistry, 
                                            sSerializer, handler);
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




// API for type registration and checking
template <typename S, typename I, typename T, typename C>
template<typename Type>
bool 
ReplayManager<S,I,T,C>::registerType(const std::string& name, const int version)
{
    return self().mTypeRegistry.template addType<Type>(name, version);
}

template <typename S, typename I, typename T, typename C>
template<typename Type>
bool 
ReplayManager<S,I,T,C>::isType(TypeBase* d)
{
    return d->id() == TypeDescriptor<Type>::id();
}

} // namespace ygg

#endif //YGG_REPLAY_MANAGER_HPP
