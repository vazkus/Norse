#ifndef YGG_SERIAL_DEVICE_HPP
#define YGG_SERIAL_DEVICE_HPP

#include "yggDeviceBase.hpp"
#include "yggTypeRegistry.hpp"
#include "yggConfig.hpp"

namespace ygg
{

template<class T, typename C>
class SerialDevice : public DeviceBase
{
    typedef typename T::MutexType   MutexType;
    typedef typename T::DeviceType  DeviceType;
    template <typename MT, typename MI, typename MC> friend class Manager;
    template <typename ST, typename SC> friend class Serializer;
    template <typename DT, typename DS, typename DI, typename CC> friend class Deserializer;

private:
    typedef typename DeviceType::Params ParamsType;
    typedef TypeBase::UnitType  UnitType;
    typedef uint32_t            SyncType;
    enum DeviceState 
    {
        DEVICE_READABLE,
        DEVICE_STOPPED,
        DEVICE_WAITING_MANIFEST,
        DEVICE_WAITING_SYNC,
        DEVICE_ERROR
    };
    enum 
    {
        SYNCH_SEQUENCE = 0x89ABCDEF
    };

private:
    SerialDevice();
    void setTypeRegistry(TypeRegistry* registry);
    void open(ParamsType& params);
    void close();
    // status checking
    bool isReadable() const;
    void setReadable();
    bool isError() const;
    void setError();
    bool isStopped() const;
    void setStopped();
    bool isWaitSynch() const;
    void setWaitSynch();
    bool isWaitManifest() const;
    void setWaitManifest();
    // writing serializable objects
    void writeData(const TypeBase* d);
    // reading serializable objects
    void readData(TypeBase*& d);

private:
    UnitType  readOneByte();
    TypeBase* buildObject(UnitType fType);
    void      writeSync(); 
    bool      readSync();

public:
    void lockWrite();
    void unlockWrite();

public:
    void write(uint32_t intd);
    void write(int32_t intd);
    void write(uint16_t intd);
    void write(int16_t intd);
    void write(uint8_t intd);
    void write(int8_t intd);
    void write(float floatd);
    void write(double doubled);
    void write(const std::string& stringd);

    void read(uint32_t& intd);
    void read(int32_t& intd);
    void read(uint16_t& intd);
    void read(int16_t& intd);
    void read(uint8_t& intd);
    void read(int8_t& intd);
    void read(float& floatd);
    void read(double& doubled);
    void read(std::string& stringd);

private:
    void write(const void* ptr, uint32_t size);
    void read(void* ptr, uint32_t size);

private:
    DeviceType    mDevice;
    MutexType     mWriteMutex;
    DeviceState   mState;
    TypeRegistry* mTypeRegistry;
    const UnitType SYNC_TYPE_ID;
};




template<typename T, typename C>
SerialDevice<T,C>::SerialDevice()
 : mState(DEVICE_STOPPED),
   mTypeRegistry(NULL),
   SYNC_TYPE_ID(std::numeric_limits<UnitType>::max())
{}

template<typename T, typename C>
void 
SerialDevice<T,C>::setTypeRegistry(TypeRegistry* registry)
{
    mTypeRegistry = registry;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::open(ParamsType& params)
{
    if(mDevice.initialize(params)) {
        mState = DEVICE_WAITING_MANIFEST;
    } else {
        mState = DEVICE_ERROR;
    }
}

template<typename T, typename C>
void 
SerialDevice<T,C>::close()
{
    mDevice.close();
    mState = DEVICE_STOPPED;
}

template<typename T, typename C>
bool 
SerialDevice<T,C>::isReadable() const 
{
    return mState == DEVICE_READABLE;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::setReadable()
{
    mState = DEVICE_READABLE;
}

template<typename T, typename C>
bool 
SerialDevice<T,C>::isError() const 
{
    return mState == DEVICE_ERROR;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::setError() 
{
    mState = DEVICE_ERROR;
}

template<typename T, typename C>
bool 
SerialDevice<T,C>::isStopped() const 
{
    return mState == DEVICE_STOPPED;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::setStopped() 
{
    mState = DEVICE_STOPPED;
}

template<typename T, typename C>
bool 
SerialDevice<T,C>::isWaitSynch() const
{
    return mState == DEVICE_WAITING_SYNC;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::setWaitSynch()
{
    mState = DEVICE_WAITING_SYNC;
}

template<typename T, typename C>
bool 
SerialDevice<T,C>::isWaitManifest() const
{
    return mState == DEVICE_WAITING_MANIFEST;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::setWaitManifest()
{
    mState = DEVICE_WAITING_MANIFEST;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::writeData(const TypeBase* d)
{
    //assert(d && d->desc());
    UnitType typeId = d->id();
    write(typeId);
    // reset the checksum, we will be using it when dumping the object
    mWriteChecksum = 0;
    d->write(*this);
    // write the calculated checksum
    write(mWriteChecksum);
}

template<typename T, typename C>
typename SerialDevice<T,C>::UnitType 
SerialDevice<T,C>::readOneByte()
{
    assert(mTypeRegistry);
    UnitType b;
    read(b);
    while (isWaitSynch() || b == SYNC_TYPE_ID) {
        if(b == SYNC_TYPE_ID) {
            if(readSync()) {
                // in case of success switch to the appropriate state
                mTypeRegistry->isManifestReceved() ? setReadable() 
                                                   : setWaitManifest();
            } else {
                // if not, continue waiting for sync
                setWaitSynch();
            }
        }
        read(b);
    }
    return b;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::readData(TypeBase*& d)
{
    d = NULL;
    UnitType fType = readOneByte();
    // if we reached here then we have a sync!
    assert(!isWaitSynch());
    d = buildObject(fType);
    // if failed to read -> start waiting for sync
    if(d == NULL) {
        setWaitSynch();
    }
}

template<typename T, typename C>
void 
SerialDevice<T,C>::writeSync() 
{
    UnitType dummy = SYNC_TYPE_ID;
    write(&dummy, sizeof(UnitType)); 
    SyncType sync = (SyncType)SYNCH_SEQUENCE;
    //write(&s, sizeof(SyncType));
    uint8_t* sb = (uint8_t*)&sync;
    uint32_t sync_len = sizeof(SyncType);
    for(uint32_t i = 0; i < sync_len; ++i) {
        write(*sb);
        sb++;
    }
}

template<typename T, typename C>
bool 
SerialDevice<T,C>::readSync()
{
    SyncType sync = (SyncType)SYNCH_SEQUENCE;
    uint8_t b;
    uint8_t* sb = (uint8_t*)&sync;
    uint32_t sync_len = sizeof(SyncType);
    for(uint32_t i = 0; i < sync_len; ++i) {
        read(b);
        if(b != *(sb)) return false;
        sb++;
    }
    return true;
}

template<typename T, typename C>
void 
SerialDevice<T,C>::lockWrite()
{
    mWriteMutex.lock();
}

template<typename T, typename C>
void 
SerialDevice<T,C>::unlockWrite()
{
    mWriteMutex.unlock();
}

template<typename T, typename C>
TypeBase* 
SerialDevice<T,C>::buildObject(UnitType fType)
{
    assert(mTypeRegistry);
    mReadChecksum = 0;
    // construct the object
    TypeBase* d = mTypeRegistry->instantiateForeignType(fType);
    // make sure the type was valid.. TBD: do a proper handling...
    if(d == NULL) {
        return NULL;
    } else {
        // read the object
        d->read(*this);
        // read the checksum
        ChecksumType computedChecksum = mReadChecksum;
        ChecksumType readChecksum;
        read(readChecksum);
        // and check the data 
        if(readChecksum != computedChecksum) {
            delete d;
            return NULL;
        }
    }
    assert(d);
    return d;
}

#if 0
////////////////////////////////////////////////////////
// Utility methods                                    //
////////////////////////////////////////////////////////
template <int Size, ConfigEndianness E>
void FixEndianness(void* v)
{
}
void swap_endiannes(int32_t& x)
{
    return;
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}
void swap_endiannes(uint32_t& x)
{
    return;
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}
#endif

////////////////////////////////////////////////////////
// Writing methods                                    //
////////////////////////////////////////////////////////
template <int Size, ConfigEndianness E>
void FixEndianness(void* v)
{
}

template<typename T, typename C>
void
SerialDevice<T,C>::write(uint32_t intd)
{
    //FixEndianness<sizeof(uint32_t), C::Endianness>(NULL);
    write(&intd, sizeof(uint32_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::write(int32_t intd)
{
    write(&intd, sizeof(int32_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::write(uint16_t intd)
{
    write(&intd, sizeof(uint16_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::write(int16_t intd)
{
    write(&intd, sizeof(int16_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::write(uint8_t intd)
{
    write(&intd, sizeof(uint8_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::write(int8_t intd)
{
    write(&intd, sizeof(int8_t));
}


template<typename T, typename C>
void
SerialDevice<T,C>::write(float floatd)
{
    write(&floatd, sizeof(float));
}

template<typename T, typename C>
void
SerialDevice<T,C>::write(double doubled)
{
    write(&doubled, sizeof(double));
}

template<typename T, typename C>
void
SerialDevice<T,C>::write(const std::string& stringd)
{
    uint32_t stringd_len = stringd.length();
    write_checksumed(stringd_len);
    write(stringd.c_str(), stringd_len);
}


////////////////////////////////////////////////////////
// Reading methods                                    //
////////////////////////////////////////////////////////
template<typename T, typename C>
void
SerialDevice<T,C>::read(uint32_t& intd)
{
    read(&intd, sizeof(uint32_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(int32_t& intd)
{
    read(&intd, sizeof(int32_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(uint16_t& intd)
{
    read(&intd, sizeof(uint16_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(int16_t& intd)
{
    read(&intd, sizeof(int16_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(uint8_t& intd)
{
    read(&intd, sizeof(uint8_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(int8_t& intd)
{
    read(&intd, sizeof(int8_t));
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(float& floatd)
{
    read(&floatd, sizeof(float));
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(double& doubled)
{
    read(&doubled, sizeof(double));
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(std::string& stringd)
{
    uint32_t stringd_len;
    read_checksumed(stringd_len);
    if(isWaitSynch()) {
        return;
    }
    char* buf = new char[stringd_len];
    read(buf, stringd_len);
    stringd.assign(buf, stringd_len);
    delete buf;
}


////////////////////////////////////////////////////////
// Low level methods                                  //
////////////////////////////////////////////////////////
template<typename T, typename C>
void
SerialDevice<T,C>::write(const void* ptr, uint32_t size)
{
    uint8_t* bptr = (uint8_t*)ptr;
    for(uint32_t i = 0; i < size; ++i) {
        if(!mDevice.write(bptr+i, 1)) {
            mState = DEVICE_ERROR;
        }
        mWriteChecksum += *(bptr+i);
    }
}

template<typename T, typename C>
void
SerialDevice<T,C>::read(void* ptr, uint32_t size) 
{
    uint8_t* bptr = (uint8_t*)ptr;
    for(uint32_t i = 0; i < size; ++i) {
        if(!mDevice.read(bptr+i, 1)) {
            mState = DEVICE_ERROR;
        }
        mReadChecksum += *(bptr+i);
    }
}

} // namespace ygg

#endif //YGG_SERIAL_DEVICE_HPP
