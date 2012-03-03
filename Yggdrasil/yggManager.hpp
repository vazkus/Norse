#ifndef YGG_MANAGER_HPP
#define YGG_MANAGER_HPP

#include "yggSerialDevice.hpp"
#include "yggSerializer.hpp"
#include "yggDeserializer.hpp"
#include <cstddef>

namespace ygg 
{

template <typename S, typename I, typename C>
class Manager
{
public:
    typedef S SystemTraits;
    typedef I InputHandler;
    typedef C Configuration;
    typedef typename S::MutexType    Mutex;
    typedef typename S::CondType     Cond;
    typedef typename S::ThreadType   Thread;
    typedef typename S::DeviceType   Device;
    typedef typename S::Utils        Utils;
    typedef ygg::SerialDevice<S,C>   SerialDevice;
    typedef ygg::Serializer<S,C>     Serializer;
    typedef typename S::DeviceParamsType   DeviceParams;
    typedef ygg::Deserializer<S,Serializer,I,C> Deserializer;
private:
    template <typename TM, ConfigManifest> class ManifestRequester;
public:
    // API used for the service initialization/start/stop.
    static void startService(DeviceParams& params, I& handler);
    static void stopService();
    static bool isFunctional();
    // API for sending receiving serializable objects.
    static void send(TypeBase* d);
    // API for type registration and checking
    template<typename T> static bool registerType(const std::string& name, 
                                               const int version);
    template<typename T> static bool isType(TypeBase* d);

private:
    static TypeRegistry  sTypeRegistry;
    static SerialDevice  sDevice;
    static Serializer*   sSerializer;
    static Deserializer* sDeserializer;
};

/////////////////////////////////////////////////////////
// static data member initialization area...           //
/////////////////////////////////////////////////////////
template<typename T, typename I, typename C> SerialDevice<T,C> Manager<T,I,C>::sDevice;
template<typename T, typename I, typename C> TypeRegistry Manager<T,I,C>::sTypeRegistry;
template<typename T, typename I, typename C> Serializer<T,C>* Manager<T,I,C>::sSerializer = NULL;
template<typename T, typename I, typename C> Deserializer<T,typename Manager<T,I,C>::Serializer,I,C>* 
                                                          Manager<T,I,C>::sDeserializer = NULL;



/////////////////////////////////////////////////////////
// function definition area...                         //
/////////////////////////////////////////////////////////
template <typename S, typename I, typename C>
void 
Manager<S, I, C>::startService(DeviceParams& params, I& handler)
{
    // TBD: design is not very good, too many data types are 
    // interconnected.. Review this.
    sTypeRegistry.initialize();
    ManifestRequester<S,C::ManifestRequired>::start();

    // open the device
    sDevice.open(params);
    if(sDevice.isError()) {
        return;
    }
    sDevice.setTypeRegistry(&sTypeRegistry);
    // construct the serializer 
    if(sSerializer == NULL) {
        sSerializer = new Serializer(sDevice);
    }
    // construct the deserializer
    if(sDeserializer == NULL) {
        // tbd: too many parameters... try to redesing...
        sDeserializer = new Deserializer(sDevice, sTypeRegistry, 
                                         *sSerializer, handler);
    }
}

template <typename S, typename I, typename C>
void 
Manager<S, I, C>::stopService() 
{
    if(sSerializer) {
        Serializer* temp = sSerializer;
        sSerializer = NULL;
        delete temp;
    }
    if(sDeserializer) {
        Deserializer* temp = sDeserializer;
        sDeserializer = NULL;
        delete temp;
    }
    sDevice.close();
}


template <typename S, typename I, typename C>
bool 
Manager<S, I, C>::isFunctional()
{
    return !sDevice.isError() && !sDevice.isStopped();
}

// API for sending receiving serializable objects.
template <typename S, typename I, typename C>
void 
Manager<S, I, C>::send(TypeBase* d)
{
    if(sSerializer && isFunctional() && sTypeRegistry.isOwnTypeEnabled(d->id())) {
        // should be a way to do this through a compile time assert.
        sSerializer->send(d);
    }
}

// API for type registration and checking
template <typename S, typename I, typename C> 
template<typename Type>
bool 
Manager<S, I, C>::registerType(const std::string& name, 
                                                          const int version)
{
    return sTypeRegistry.addType<Type>(name, version);
}

template <typename S, typename I, typename C> 
template<typename T>
bool 
Manager<S, I, C>::isType(TypeBase* d)
{
    return d->id() == TypeDescriptor<T>::id();
}

/////////////////////////////////////////////////////////
//   Partial specialization of the class ManifestRe-   //
//   quester for MANIFEST_REQUIRED configuration       //  
/////////////////////////////////////////////////////////
template <typename S, typename I, typename C> 
template <typename DT>
class Manager<S, I, C>::ManifestRequester<DT,MANIFEST_REQUIRED>
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
        if(Manager<S,I,C>::sTypeRegistry.isManifestReceved()) {
            return true;
        }
        Manager<S,I,C>::send(new SystemCmdData(SystemCmdData::CMD_MANIFEST_REQUEST));
        Thread::sleepMilliseconds(C::ManifestRequestMs);
        return false;
    }
};

/////////////////////////////////////////////////////////
//   Partial specialization of the class Manifest      //
//   Requester for MANIFEST_IGNORE configuration       //  
/////////////////////////////////////////////////////////
template <typename S, typename I, typename C> 
template <typename TH>
class Manager<S, I, C>::ManifestRequester<TH,MANIFEST_IGNORE>
{
    typedef Manager<S,I,C> M;
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
