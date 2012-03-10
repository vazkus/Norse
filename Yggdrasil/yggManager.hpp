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
class Manager
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
private:
    template <typename TM, ConfigManifest> class ManifestRequester;
public:
    // API used for the service initialization/start/stop.
    static void startService(Transport& transport, L& logger, I& handler);
    static void stopService();
    // API for sending receiving serializable objects.
    static void send(TypeBase* d);
    // API for type registration and checking
    template<typename T> static bool registerType(const std::string& name, 
                                                  const int version);
    template<typename T> static bool isType(TypeBase* d);

private:
    static TypeRegistry  sTypeRegistry;
    static Serializer*   sSerializer;
    static Deserializer* sDeserializer;
};

/////////////////////////////////////////////////////////
// static data member initialization area...           //
/////////////////////////////////////////////////////////
template<typename S, typename I, typename L, typename C> 
TypeRegistry Manager<S,I,L,C>::sTypeRegistry;
template<typename S, typename I, typename L, typename C> 
Serializer<S,C>* Manager<S,I,L,C>::sSerializer = NULL;
template<typename S, typename I, typename L, typename C> 
Deserializer<S,typename Manager<S,I,L,C>::Serializer,I,L,C>* Manager<S,I,L,C>::sDeserializer = NULL;


/////////////////////////////////////////////////////////
// function definition area...                         //
/////////////////////////////////////////////////////////
template <typename S, typename I, typename L, typename C>
void 
Manager<S,I,L,C>::startService(Transport& transport, L& logger, I& handler)
{
    // TBD: design is not very good, too many data types are 
    // interconnected.. Review this.
    sTypeRegistry.initialize();
    ManifestRequester<S,C::ManifestRequired>::start();

    // start the transport
    transport.start();
    logger.start();
    if(transport.isError()) {
        return;
    }
    transport.setTypeRegistry(&sTypeRegistry);
    logger.setTypeRegistry(&sTypeRegistry);
    // construct the serializer 
    if(sSerializer == NULL) {
        sSerializer = new Serializer(transport);
    }
    // construct the deserializer
    if(sDeserializer == NULL) {
        sDeserializer = new Deserializer(transport, sTypeRegistry, 
                                         logger, *sSerializer, handler);
    }
}

template <typename S, typename I, typename L, typename C>
void 
Manager<S,I,L,C>::stopService() 
{
    if(sSerializer) {
        sSerializer->stop();
        Serializer* temp = sSerializer;
        sSerializer = NULL;
        delete temp;
    }
    if(sDeserializer) {
        sDeserializer->stop();
        Deserializer* temp = sDeserializer;
        sDeserializer = NULL;
        delete temp;
    }
    sTypeRegistry.reset();
}

// API for sending receiving serializable objects.
template <typename S, typename I, typename L, typename C>
void 
Manager<S,I,L,C>::send(TypeBase* d)
{
    if(sSerializer && sSerializer->isFunctional() &&
       sTypeRegistry.isOwnTypeEnabled(d->id())) {
        // should be a way to do this through a compile time assert.
        sSerializer->send(d);
    }
}

// API for type registration and checking
template <typename S, typename I, typename L, typename C>
template<typename Type>
bool 
Manager<S,I,L,C>::registerType(const std::string& name, const int version)
{
    return sTypeRegistry.addType<Type>(name, version);
}

template <typename S, typename I, typename L, typename C>
template<typename Type>
bool 
Manager<S,I,L,C>::isType(TypeBase* d)
{
    return d->id() == TypeDescriptor<Type>::id();
}

/////////////////////////////////////////////////////////
//   Partial specialization of the class ManifestRe-   //
//   quester for MANIFEST_REQUIRED configuration       //  
/////////////////////////////////////////////////////////
template <typename S, typename I, typename L, typename C>
template <typename DT>
class Manager<S,I,L,C>::ManifestRequester<DT,MANIFEST_REQUIRED>
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
        if(Manager<S,I,L,C>::sTypeRegistry.isManifestReceved()) {
            return true;
        }
        Manager<S,I,L,C>::send(new SystemCmdData(SystemCmdData::CMD_MANIFEST_REQUEST));
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
class Manager<S,I,L,C>::ManifestRequester<TH,MANIFEST_IGNORE>
{
    typedef Manager<S,I,L,C> M;
public:
    static void start() 
    {
        TypeRegistry::TypeDescriptorConstIt dit = M::sTypeRegistry::descriptorBegin();
        TypeRegistry::TypeDescriptorConstIt edit = M::sTypeRegistry::descriptorEnd();
        for(;  dit != edit; ++dit) {
            typename M::UnitType tId = dit->descriptor->typeId();
            M::sTypeRegistry::acceptType(tId, tId);
        }
        M::sTypeRegistry::setManifestReceived(true);
    }
};
} // namespace ygg

#endif //YGG_MANAGER_HPP
