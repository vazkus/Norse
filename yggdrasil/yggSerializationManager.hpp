#ifndef YGG_MANAGER_HPP
#define YGG_MANAGER_HPP

#include "yggTransport.hpp"
#include "yggTransportImpl.hpp"
#include "yggSerializer.hpp"
#include "yggDeserializer.hpp"
#include <cstddef>

namespace ygg 
{

template <typename S, typename I, typename L, typename C>
class SerializationManager
{
public:
    typedef S SystemTraits;
    typedef I InputHandler;
    typedef C Configuration;
    typedef L Logger;
    typedef ConfiguredTransport<C> Transport;
    typedef typename S::MutexType  Mutex;
    typedef typename S::CondType   Condition;
    typedef typename S::ThreadType Thread;
    typedef typename S::DeviceType Device;
    typedef typename S::Utils      Utils;
    typedef ygg::Serializer<S,C>   Serializer;
    typedef typename S::DeviceParamsType   DeviceParams;
    typedef ygg::Deserializer<S,Serializer,I,L,C> Deserializer;

public:
    // API used for the service initialization/start/stop.
    static void startService(Transport& transport, I& handler);
    static void stopService();
    // API usef for logging
    static void startLogger(L& logger);
    static void stopLogger();
    // API for sending serializable objects.
    static void send(TypeBase* d);
    // API for type registration and checking
    template<typename T> static bool registerType(const std::string& name, 
                                                  const int version);
    template<typename T> static bool isType(TypeBase* d);

private:
    template <typename TM, ConfigManifest> class ManifestRequester;
    SerializationManager();
    static SerializationManager& self();
private:
    TypeRegistry  mTypeRegistry;
    Serializer*   mSerializer;
    Deserializer* mDeserializer;
};

template <typename S, typename I, typename L, typename C>
SerializationManager<S,I,L,C>::SerializationManager()
{
}

template <typename S, typename I, typename L, typename C>
SerializationManager<S,I,L,C>& 
SerializationManager<S,I,L,C>::self()
{
    static SerializationManager<S,I,L,C> sManager;
    return sManager;
}

/////////////////////////////////////////////////////////
// function definition area...                         //
/////////////////////////////////////////////////////////
template <typename S, typename I, typename L, typename C>
void 
SerializationManager<S,I,L,C>::startService(Transport& transport, I& handler)
{
    // TBD: design is not very good, too many data types are 
    // interconnected.. Review this.
    self().mTypeRegistry.initialize();
    ManifestRequester<S,C::ManifestRequired>::start();

    // start the transport
    transport.start();
    if(transport.isError()) {
        return;
    }
    transport.setTypeRegistry(&self().mTypeRegistry);
    // construct the serializer 
    if(self().mSerializer == NULL) {
        self().mSerializer = new Serializer(transport);
    }
    // construct the deserializer
    if(self().mDeserializer == NULL) {
        self().mDeserializer = new Deserializer(transport, self().mTypeRegistry, 
                                                *self().mSerializer, handler);
    }
}

template <typename S, typename I, typename L, typename C>
void 
SerializationManager<S,I,L,C>::startLogger(Logger& logger)
{
    // TBD: add execution status logging...
    if(self().mDeserializer == NULL) {
        return;
    }
    // set type registry...
    logger.setTypeRegistry(&self().mTypeRegistry);
    // write manifest
    logger.start();
    TypeBase* d = self().mTypeRegistry.extractManifest();
    logger.writeData(d);
    delete d;
    logger.stop();
    // attach to deserializer
    self().mDeserializer->setLogger(logger);
    self().mDeserializer->getLogger().start();
}

template <typename S, typename I, typename L, typename C>
void 
SerializationManager<S,I,L,C>::stopLogger() 
{
    self().mDeserializer()->getLogger().stop();
}

template <typename S, typename I, typename L, typename C>
void 
SerializationManager<S,I,L,C>::stopService() 
{
    if(self().mSerializer) {
        self().mSerializer->stop();
        Serializer* temp = self().mSerializer;
        self().mSerializer = NULL;
        delete temp;
    }
    if(self().mDeserializer) {
        self().mDeserializer->stop();
        Deserializer* temp = self().mDeserializer;
        self().mDeserializer = NULL;
        delete temp;
    }
    self().mTypeRegistry.reset();
}

// API for sending receiving serializable objects.
template <typename S, typename I, typename L, typename C>
void 
SerializationManager<S,I,L,C>::send(TypeBase* d)
{
    if(self().mSerializer && self().mSerializer->isFunctional() &&
       self().mTypeRegistry.isOwnTypeEnabled(d->id())) {
        // should be a way to do this through a compile time assert.
        self().mSerializer->send(d);
    }
}

// API for type registration and checking
template <typename S, typename I, typename L, typename C>
template<typename Type>
bool 
SerializationManager<S,I,L,C>::registerType(const std::string& name, const int version)
{
    return self().mTypeRegistry.template addType<Type>(name, version);
}

template <typename S, typename I, typename L, typename C>
template<typename Type>
bool 
SerializationManager<S,I,L,C>::isType(TypeBase* d)
{
    return d->id() == TypeDescriptor<Type>::id();
}

/////////////////////////////////////////////////////////
//   Partial specialization of the class ManifestRe-   //
//   quester for MANIFEST_REQUIRED configuration       //  
/////////////////////////////////////////////////////////
template <typename S, typename I, typename L, typename C>
template <typename DT>
class SerializationManager<S,I,L,C>::ManifestRequester<DT,MANIFEST_REQUIRED>
{
    typedef TypeRegistry::SystemCmdData SystemCmdData;
    typedef typename S::ThreadType      Thread;
public:
    static void start() 
    {
        static Thread mRequester("ManifestRequester", 512, C::BasePriority, requestFunc);
    }
private:
    static bool requestFunc(void*)
    {
        if(SerializationManager<S,I,L,C>::self().mTypeRegistry.isManifestReceved()) {
            return true;
        }
        SerializationManager<S,I,L,C>::send(new SystemCmdData(SystemCmdData::CMD_MANIFEST_REQUEST));
        Thread::sleepMilliseconds(C::ManifestRequestMs);
        return false;
    }
};

/////////////////////////////////////////////////////////
//   Partial specialization of the class Manifest      //
//   Requester for MANIFEST_IGNORE configuration       //  
/////////////////////////////////////////////////////////
template <typename S, typename I, typename L, typename C>
template <typename TH>
class SerializationManager<S,I,L,C>::ManifestRequester<TH,MANIFEST_IGNORE>
{
    typedef SerializationManager<S,I,L,C> SM;
public:
    static void start() 
    {
        TypeRegistry::TypeDescriptorConstIt dit = SM::self().mTypeRegistry.descriptorBegin();
        TypeRegistry::TypeDescriptorConstIt edit = SM::self().mTypeRegistry.descriptorEnd();
        for(;  dit != edit; ++dit) {
            typename SM::UnitType tId = dit->descriptor->typeId();
            SM::self().mTypeRegistry.acceptType(tId, tId);
        }
        SM::self().mTypeRegistry.setManifestReceived(true);
    }
};
} // namespace ygg

#endif //YGG_MANAGER_HPP
