#ifndef YGG_DESERIALIZER_HPP
#define YGG_DESERIALIZER_HPP

#include "yggQueue.hpp"
#include "yggTypes.hpp"
#include "yggSerialDevice.hpp"
#include "yggConfig.hpp"


namespace ygg {

template<typename T, typename S, typename I, typename C>
class Deserializer
{
    typedef typename T::MutexType   MutexType;
    typedef typename T::DeviceType  DeviceType;
    typedef SerialDevice<T, C>      SerialDeviceType;
private:
    template <typename MT, typename MI, typename MC> friend class Manager;
    Deserializer(SerialDeviceType& device, 
                 TypeRegistry& registry, 
                 S& serializer,
                 I& handler);
private:
    template<typename TH, ConfigCommunication>
    class Helper 
    {
    public:
        Helper(Deserializer<T,S,I,C>& ds);
    };
private:
    SerialDeviceType& mDevice;
    TypeRegistry&     mTypeRegistry;
    S&  mSerializer;
    I&  mHandler;
    Helper<T,C::Deserialization> mHelper;
};

template <typename T, typename S, typename I, typename C>
Deserializer<T, S, I, C>::Deserializer(SerialDeviceType& device, 
                                         TypeRegistry& registry, 
                                         S& serializer,
                                         I& handler)
 : mDevice(device),
   mTypeRegistry(registry),
   mSerializer(serializer),
   mHandler(handler),
   mHelper(*this)
{
}


/////////////////////////////////////////////////////////
//   partial specialization of the helper class for    //
//   COMMUNICATION_BLOCKING configuration              //  
/////////////////////////////////////////////////////////
template <typename T, typename S, typename I, typename C>
template <typename TH>
class Deserializer<T,S,I,C>::Helper<TH,COMMUNICATION_BLOCKING>
{
    template <typename MT, typename MI, typename MC> friend class Manager;
    typedef typename TypeRegistry::ManifestData  ManifestDataType;
    typedef typename TypeRegistry::SystemCmdData SysCmdDataType;
public:
    Helper(Deserializer<T,S,I,C>& ds);
    void reset();
private:
    Deserializer<T,S,I,C>& mOwner;
};

template <typename T, typename S, typename I, typename C>
template <typename TH>
Deserializer<T,S,I,C>::Helper<TH, COMMUNICATION_BLOCKING>::Helper(Deserializer<T,S,I,C>& ds)
  : mOwner(ds)
{
    // find a proper break condition...
    while(true) {
        TypeBase* d = NULL;
        mOwner.mDevice.readData(d);
        if(d == NULL) {
            continue;
        }
        if(d->id() == TypeDescriptor<ManifestDataType>::id()) {
            ManifestDataType* md = (ManifestDataType*)d;
            mOwner.mTypeRegistry.applyManifest(md);
        } else
        if(d->id() == TypeDescriptor<SysCmdDataType>::id()) {
            SysCmdDataType* sd = (SysCmdDataType*)d;
            if(*sd == SysCmdDataType::CMD_MANIFEST_REQUEST) {
                mOwner.mSerializer.reset();
                mOwner.mSerializer.send(mOwner.mTypeRegistry.extractManifest());
            }
        } else
        if(mOwner.mTypeRegistry.isOwnTypeEnabled(d->id())) {
            mOwner.mHandler.process(d);
        }
        delete d;
    }
}

template <typename T, typename S, typename I, typename C>
template <typename TH>
void
Deserializer<T,S,I,C>::Helper<TH, COMMUNICATION_BLOCKING>::reset()
{
}



/////////////////////////////////////////////////////////
//   partial specialization of the helper class for    //
//   COMMUNICATION_NONBLOCKING configuration           //  
/////////////////////////////////////////////////////////
template <typename T, typename S, typename I, typename C>
template <typename TH>
class Deserializer<T,S,I,C>::Helper<TH,COMMUNICATION_NONBLOCKING>
{
    template <typename MT, typename MI, typename MC> friend class Manager;
    typedef typename T::MutexType    MutexType;
    typedef typename T::CondType     CondType;
    typedef typename T::ThreadType   ThreadType;
    typedef Queue<TypeBase, MutexType, CondType> QueueType;
    typedef typename QueueType::TypeList         QueueTypeList;
    typedef typename TypeRegistry::ManifestData  ManifestDataType;
    typedef typename TypeRegistry::SystemCmdData SysCmdDataType;
public:
    Helper(Deserializer<T,S,I,C>& ds);
    void reset();
    static bool deserializerFunc(void*);
    static bool inputHanderFunc(void*);
private:
    Deserializer<T,S,I,C>& mOwner;
    QueueType   mInputQueue;
    ThreadType  mDeserializer;
    ThreadType  mHandlerThread;
};

template <typename T, typename S, typename I, typename C>
template <typename TH>
Deserializer<T,S,I,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::Helper(Deserializer<T,S,I,C>& ds)
  : mOwner(ds),
    mInputQueue(C::InputQueueSize),
    mDeserializer("Deserializer", 1536, C::BasePriority+1, deserializerFunc, NULL, this),
    mHandlerThread("InputHandler", 1536, C::BasePriority+2, inputHanderFunc, NULL, this)
{
}

template <typename T, typename S, typename I, typename C>
template <typename TH>
void
Deserializer<T,S,I,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::reset()
{
    mOwner.mInputQueue.clear();
}

template <typename T, typename S, typename I, typename C>
template <typename TH>
bool 
Deserializer<T,S,I,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::deserializerFunc(void* param)
{
    Helper<TH,COMMUNICATION_NONBLOCKING>* h = (Helper<TH,COMMUNICATION_NONBLOCKING>*)param;
    TypeBase* d = NULL;
    h->mOwner.mDevice.readData(d);
    if(d != NULL) {
        assert(h->mOwner.mTypeRegistry.isOwnTypeEnabled(d->id())); 
        h->mInputQueue.push(d);
    }    
    return false;
}

template <typename T, typename S, typename I, typename C>
template <typename TH>
bool 
Deserializer<T,S,I,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::inputHanderFunc(void* param)
{
    Helper<TH,COMMUNICATION_NONBLOCKING>* h = (Helper<TH,COMMUNICATION_NONBLOCKING>*)param;
    QueueTypeList dlist;
    h->mInputQueue.popAll(dlist);
    typename QueueTypeList::iterator it = dlist.begin();
    typename QueueTypeList::iterator eit = dlist.end();
    for(; it != eit; ++it) {
        TypeBase* d = *it;
        if(d->id() == TypeDescriptor<ManifestDataType>::id()) {
            ManifestDataType* md = (ManifestDataType*)d;
            h->mOwner.mTypeRegistry.applyManifest(md);
        } else
        if(d->id() == TypeDescriptor<SysCmdDataType>::id()) {
            SysCmdDataType* sd = (SysCmdDataType*)d;
            if(*sd == SysCmdDataType::CMD_MANIFEST_REQUEST) {
                h->mOwner.mSerializer.reset();
                h->mOwner.mSerializer.send(h->mOwner.mTypeRegistry.extractManifest());
            }
        } else
        if(h->mOwner.mTypeRegistry.isOwnTypeEnabled(d->id())) {
            h->mOwner.mHandler.process(d);
        }
        delete d;
    }
    return false;
}

} // namespace ygg

#endif //YGG_DESERIALIZER_HPP
