#ifndef YGG_SERIALIZER_HPP
#define YGG_SERIALIZER_HPP

#include "yggQueue.hpp"
#include "yggTypes.hpp"
#include "yggTransport.hpp"
#include "yggConfig.hpp"


namespace ygg {

template<typename T, typename C>
class Serializer
{
    typedef typename T::MutexType   MutexType;
public:
    Serializer(Transport& transport);
    bool isFunctional();
    void send(TypeBase* d);
    void reset();
    void stop();
private:
    template<typename TH, ConfigCommunication>
    class Helper 
    {
    public:
        Helper(const Serializer<T,C>* s);
        void send(TypeBase* d);
        void reset();
    };
private:
    Transport&  mTransport;
    Helper<T,C::Serialization> mHelper;
};

template<>
class Serializer<DummyType, DummyType>
{
public:
    Serializer(Transport&)
    {}
    bool isFunctional()
    {
        return true;
    }
    void send(TypeBase*)
    {}
    void reset()
    {}
    void stop()
    {}
};

typedef Serializer<DummyType, DummyType> DummySerializer;


/////////////////////////////////////////////////////////
//   Function definitions for the class Serializer     //
/////////////////////////////////////////////////////////
template <typename T, typename C>
Serializer<T,C>::Serializer(Transport& transport)
 : mTransport(transport),
   mHelper(*this)
{
}

template <typename T, typename C>
bool 
Serializer<T, C>::isFunctional()
{
    return !mTransport.isError() && !mTransport.isStopped();

}

template <typename T, typename C>
void
Serializer<T,C>::send(TypeBase* d)
{
    mHelper.send(d);
}

template <typename T, typename C>
void 
Serializer<T,C>::reset()
{
    mHelper.reset();
}

template <typename T, typename C>
void 
Serializer<T,C>::stop()
{
    mTransport.stop();
}


/////////////////////////////////////////////////////////
//   Partial specialization of the helper class for    //
//   COMMUNICATION_BLOCKING configuration              //  
/////////////////////////////////////////////////////////
template <typename T, typename C>
template <typename TH>
class Serializer<T,C>::Helper<TH,COMMUNICATION_BLOCKING>
{
    template <typename MT, typename MI, typename ML, typename MC> friend class Manager;
public:
    Helper(Serializer<T,C>& s);
    void send(TypeBase* d);
    void reset();
private:
    Serializer<T,C>& mOwner;
};

template <typename T, typename C>
template <typename TH>
Serializer<T,C>::Helper<TH, COMMUNICATION_BLOCKING>::Helper(Serializer<T,C>& s)
  : mOwner(s)
{
}

template <typename T, typename C>
template <typename TH>
void
Serializer<T,C>::Helper<TH, COMMUNICATION_BLOCKING>::send(TypeBase* d)
{
    mOwner.mTransport.writeData(d);
    delete d;
}

template <typename T, typename C>
template <typename TH>
void 
Serializer<T,C>::Helper<TH, COMMUNICATION_BLOCKING>::reset()
{
}


/////////////////////////////////////////////////////////
//   Partial specialization of the helper class for    //
//   COMMUNICATION_NONBLOCKING configuration           //  
/////////////////////////////////////////////////////////
template <typename T, typename C>
template <typename TH>
class Serializer<T,C>::Helper<TH,COMMUNICATION_NONBLOCKING>
{
    template <typename MT, typename MI, typename ML, typename MC> friend class Manager;
    typedef typename T::MutexType    MutexType;
    typedef typename T::CondType     CondType;
    typedef typename T::ThreadType   ThreadType;
    typedef Queue<TypeBase,MutexType,CondType> QueueType;
    typedef typename QueueType::TypeList       QueueTypeList;
public:
    Helper(Serializer<T,C>& s);
    void send(TypeBase* d);
    void reset();
    static bool serializerFunc(void*);
private:
    Serializer<T,C>& mOwner;
    QueueType   mOutputQueue;
    ThreadType  mSerializerThread;
};


template <typename T, typename C>
template <typename TH>
Serializer<T,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::Helper(Serializer<T,C>& s)
  : mOwner(s),
    mOutputQueue(C::OutputQueueSize),
    mSerializerThread("Serializer", 1524, C::BasePriority+1, serializerFunc, NULL, this)
{
}

template <typename T, typename C>
template <typename TH>
void
Serializer<T,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::send(TypeBase* d)
{
    mOutputQueue.push(d);
}

template <typename T, typename C>
template <typename TH>
void 
Serializer<T,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::reset()
{
    mOutputQueue.clear();
}

template <typename T, typename C>
template <typename TH>
bool
Serializer<T,C>::Helper<TH, COMMUNICATION_NONBLOCKING>::serializerFunc(void* param)
{
    Helper<TH,COMMUNICATION_NONBLOCKING>* h = (Helper<TH,COMMUNICATION_NONBLOCKING>*)param;
    // pop the next object
    TypeBase* d = h->mOutputQueue.pop();
    //assert(d && d->desc());
    // write it into the device
    h->mOwner.mTransport.writeData(d);
    // data is sent, we can destroy the object
    delete d;
    return false;
}

} // namespace ygg
#endif //YGG_SERIALIZER_HPP
