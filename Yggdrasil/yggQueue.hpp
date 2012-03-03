#ifndef YGG_DATA_QUEUE_HPP
#define YGG_DATA_QUEUE_HPP

#include <cstddef>
#include <list>

namespace ygg 
{

template <class Type, class MutexType, class CondType>
class Queue
{
public:
    typedef std::list<Type*> TypeList;
public:
    Queue(uint32_t maxSize);
    Type* pop();
    bool push(Type* dt);
    void popAll(TypeList& dlist);
    void clear();
private:
    MutexType mMutex;
    CondType  mCond;
    TypeList  mQueue;
    uint32_t  mMaxSize;
};

template <class T, class M, class C>
Queue<T,M,C>::Queue(uint32_t maxSize)
 : mCond(mMutex),
   mMaxSize(maxSize)
{ }

template <class T, class M, class C>
T* 
Queue<T,M,C>::pop()
{
    mMutex.lock();
    while(mQueue.empty()) {
        mCond.wait();
    } 
    T* dt = mQueue.front();
    mQueue.pop_front();  
    mMutex.unlock();
    return dt;
}

template <class T, class M, class C>
bool 
Queue<T,M,C>::push(T* dt)
{
    bool ok = false;
    mMutex.lock();
    if(mQueue.size() < mMaxSize) {
        mQueue.push_back(dt);
        mCond.signal();
        ok = true;
    }
    mMutex.unlock();
    return ok;
}

template <class T, class M, class C>
void 
Queue<T,M,C>::popAll(TypeList& dlist) 
{
    mMutex.lock();
    while(mQueue.empty()) {
        mCond.wait();
    } 
    dlist.swap(mQueue);
    mMutex.unlock();
}

template <class T, class M, class C>
void 
Queue<T,M,C>::clear()
{
    mMutex.lock();
    typename TypeList::iterator dit = mQueue.begin();
    typename TypeList::iterator edit = mQueue.end();
    for(; dit != edit; ++dit) {
        delete *dit;
    }
    mQueue.clear();
    mMutex.unlock();
}   

} //namespace ygg

#endif //YGG_DATA_QUEUE_HPP

