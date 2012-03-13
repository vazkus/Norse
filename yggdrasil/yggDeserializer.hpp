#ifndef YGG_DESERIALIZER_HPP
#define YGG_DESERIALIZER_HPP

#include "yggQueue.hpp"
#include "yggTypes.hpp"
#include "yggTransport.hpp"
#include "yggConfig.hpp"


namespace ygg {

template<typename T, typename S, typename I, typename L, typename C>
class Deserializer
{
    typedef typename T::MutexType   MutexType;
public:
    Deserializer(Transport& transport, 
                 S& serializer,
                 I& handler);
    bool isFunctional();
    void stop();
    void sendManifestRequest();
    // logger accessor/mutator API 
    L&   getLogger();
    void setLogger(L& logger);
private:
    template<typename TH, ConfigCommunication>
    class Helper 
    {
    public:
        Helper(Deserializer<T,S,I,L,C>& ds);
    };
private:
    Transport&    mTransport;
    L   mLogger;  
    S&  mSerializer;
    I&  mHandler;
    Helper<T,C::Deserialization> mHelper;
};


/////////////////////////////////////////////////////////
// function definition area...                         //
/////////////////////////////////////////////////////////
template <typename T, typename S, typename I, typename L, typename C>
Deserializer<T,S,I,L,C>::Deserializer(Transport& transport, 
                                      S& serializer,
                                      I& handler)
 : mTransport(transport),
   mSerializer(serializer),
   mHandler(handler),
   mHelper(*this)
{
}

template <typename T, typename S, typename I, typename L, typename C>
bool 
Deserializer<T,S,I,L,C>::isFunctional()
{
    return !mTransport.isError() && !mTransport.isStopped();
}

template <typename T, typename S, typename I, typename L, typename C>
void
Deserializer<T,S,I,L,C>::stop()
{
    mTransport.stop();
}

template <typename T, typename S, typename I, typename L, typename C>
void
Deserializer<T,S,I,L,C>::sendManifestRequest()
{
    mSerializer.reset();
    mSerializer.send(TypeRegistry::extractManifest());
}

template <typename T, typename S, typename I, typename L, typename C>
L&
Deserializer<T,S,I,L,C>::getLogger()
{
    return mLogger;
}

template <typename T, typename S, typename I, typename L, typename C>
void
Deserializer<T,S,I,L,C>::setLogger(L& logger)
{
    // make sure this is thread safe!
    mLogger.swap(logger);
}

/////////////////////////////////////////////////////////
//   partial specialization of the helper class for    //
//   COMMUNICATION_BLOCKING configuration              //  
/////////////////////////////////////////////////////////
template <typename T, typename S, typename I, typename L, typename C>
template <typename TH>
class Deserializer<T,S,I,L,C>::Helper<TH,COMMUNICATION_BLOCKING>
{
    template <typename MT, typename MI, typename ML, typename MC> friend class Manager;
    typedef typename TypeRegistry::ManifestData  ManifestDataType;
    typedef typename TypeRegistry::SystemCmdData SysCmdDataType;
public:
    Helper(Deserializer<T,S,I,L,C>& ds);
    void reset();
private:
    Deserializer<T,S,I,L,C>& mOwner;
};

template <typename T, typename S, typename I, typename L, typename C>
template <typename TH>
Deserializer<T,S,I,L,C>::Helper<TH, COMMUNICATION_BLOCKING>::Helper(Deserializer<T,S,I,L,C>& ds)
  : mOwner(ds)
{
    // find a proper break condition...
    while(true) {
        TypeBase* d = NULL;
        mOwner.mTransport.deserialize(d);
        if(d == NULL) {
            continue;
        }
        if(d->id() == TypeDescriptor<ManifestDataType>::id()) {
            ManifestDataType* md = (ManifestDataType*)d;
            TypeRegistry::applyManifest(md);
        } else
        if(d->id() == TypeDescriptor<SysCmdDataType>::id()) {
            SysCmdDataType* sd = (SysCmdDataType*)d;
            if(*sd == SysCmdDataType::CMD_MANIFEST_REQUEST) {
                mOwner.sendManifestRequest();
            }
        } else
        if(TypeRegistry::isOwnTypeEnabled(d->id())) {
            mOwner.mHandler.process(d);
            // write the object to the log...
            mOwner.mLogger.serialize(d);
        }
        delete d;
    }
}

template <typename T, typename S, typename I, typename L, typename C>
template <typename TH>
void
Deserializer<T,S,I,L,C>::Helper<TH, COMMUNICATION_BLOCKING>::reset()
{
}



/////////////////////////////////////////////////////////
//   partial specialization of the helper class for    //
//   COMMUNICATION_NONBLOCKING configuration           //  
/////////////////////////////////////////////////////////
template <typename T, typename S, typename I, typename L, typename C>
template <typename TH>
class Deserializer<T,S,I,L,C>::Helper<TH,COMMUNICATION_NONBLOCKING>
{
    template <typename MT, typename MI, typename ML, typename MC> friend class Manager;
    typedef typename T::MutexType    MutexType;
    typedef typename T::CondType     CondType;
    typedef typename T::ThreadType   ThreadType;
    typedef Queue<TypeBase, MutexType, CondType> QueueType;
    typedef typename QueueType::TypeList         QueueTypeList;
    typedef typename TypeRegistry::ManifestData  ManifestDataType;
    typedef typename TypeRegistry::SystemCmdData SysCmdDataType;
public:
    Helper(Deserializer<T,S,I,L,C>& ds);
    void reset();
    static bool deserializerFunc(void*);
    static bool inputHanderFunc(void*);
private:
    Deserializer<T,S,I,L,C>& mOwner;
    QueueType   mInputQueue;
    ThreadType  mDeserializer;
    ThreadType  mHandlerThread;
};

template <typename T, typename S, typename I, typename L, typename C>
template <typename TH>
Deserializer<T,S,I,L,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::Helper(Deserializer<T,S,I,L,C>& ds)
  : mOwner(ds),
    mInputQueue(C::InputQueueSize),
    mDeserializer("Deserializer", 1536, C::BasePriority+1, deserializerFunc, NULL, this),
    mHandlerThread("InputHandler", 1536, C::BasePriority+2, inputHanderFunc, NULL, this)
{
}

template <typename T, typename S, typename I, typename L, typename C>
template <typename TH>
void
Deserializer<T,S,I,L,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::reset()
{
    mOwner.mInputQueue.clear();
}

template <typename T, typename S, typename I, typename L, typename C>
template <typename TH>
bool 
Deserializer<T,S,I,L,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::deserializerFunc(void* param)
{
    Helper<TH,COMMUNICATION_NONBLOCKING>* h = (Helper<TH,COMMUNICATION_NONBLOCKING>*)param;
    TypeBase* d = NULL;
    h->mOwner.mTransport.deserialize(d);
    if(d != NULL) {
        assert(TypeRegistry::isOwnTypeEnabled(d->id())); 
        h->mInputQueue.push(d);
    }    
    return false;
}

template <typename T, typename S, typename I, typename L, typename C>
template <typename TH>
bool 
Deserializer<T,S,I,L,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::inputHanderFunc(void* param)
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
            TypeRegistry::applyManifest(md);
        } else
        if(d->id() == TypeDescriptor<SysCmdDataType>::id()) {
            SysCmdDataType* sd = (SysCmdDataType*)d;
            if(*sd == SysCmdDataType::CMD_MANIFEST_REQUEST) {
                h->mOwner.sendManifestRequest();
            }
        } else
        if(TypeRegistry::isOwnTypeEnabled(d->id())) {
            h->mOwner.mHandler.process(d);
            // write the object to the log...
            h->mOwner.mLogger.serialize(d);
        }
        delete d;
    }
    return false;
}

} // namespace ygg

#endif //YGG_DESERIALIZER_HPP
