#ifndef YGG_TRANSPORT_IMPL_HPP
#define YGG_TRANSPORT_IMPL_HPP

#include "yggDeviceBase.hpp"
#include "yggTypeRegistry.hpp"
#include <cassert>


namespace ygg
{

inline
Transport::Transport()
 : mDevice(NULL),
   mState(DEVICE_STOPPED),
   mTypeRegistry(NULL),
   mReadChecksum(0),
   mWriteChecksum(0)
{}

inline void 
Transport::setTypeRegistry(TypeRegistry* registry)
{
    mTypeRegistry = registry;
}

inline void 
Transport::start(DeviceBase* device)
{
    mDevice = device;
    if(mDevice->isOpen()) {
        mState = DEVICE_WAITING_SYNC;
    } else {
        mState = DEVICE_ERROR;
    }
}

inline void 
Transport::stop()
{
    mState = DEVICE_STOPPED;
}

inline bool 
Transport::isReadable() const 
{
    return mState == DEVICE_READABLE;
}

inline void 
Transport::setReadable()
{
    mState = DEVICE_READABLE;
}

inline bool 
Transport::isError() const 
{
    return mState == DEVICE_ERROR;
}

inline void 
Transport::setError() 
{
    mState = DEVICE_ERROR;
}

inline bool 
Transport::isStopped() const 
{
    return mState == DEVICE_STOPPED;
}

inline void 
Transport::setStopped() 
{
    mState = DEVICE_STOPPED;
}

inline bool 
Transport::isWaitSync() const
{
    return mState == DEVICE_WAITING_SYNC;
}

inline void 
Transport::setWaitSync()
{
    mState = DEVICE_WAITING_SYNC;
}

inline void 
Transport::writeData(const TypeBase* d)
{
    //assert(d && d->desc());
    UnitType typeId = d->id();
    // write the synchronization byte
    write((UnitType)SYNC_BYTE);
    // write the type
    write(typeId);
    // write the checksum
    UnitType cs = 255 - typeId - SYNC_BYTE;
    write(cs);
    // reset the checksum, we will be using it when dumping the object
    mWriteChecksum = 0;
    d->write(*this);
    // write the calculated checksum
    write(mWriteChecksum);
}

inline Transport::UnitType 
Transport::readObjectType()
{
    assert(mTypeRegistry);
    UnitType s, t, cs;
    // read the first byte, we hope this is the sync.
    read(s);
    while (isWaitSync()) {
        if(s == SYNC_BYTE) {
            // ok check the next one, it should be the data type byte
            read(t);
            if(!mTypeRegistry->isForeignTypeEnabled(t)) {
                // nope, continue search
                s = t;
                continue;
            }
            // ok, so far so good, read the checksum
            read(cs);
            if(cs + s + t != 255) {
                // checksum didn't match, continue from here
                s = cs;
                continue;
            }
            // we are good to go!
            setReadable();
            break;
        }
        read(s);
    }
    return t;
}

inline void 
Transport::readData(TypeBase*& d)
{
    d = NULL;
    UnitType fType = readObjectType();
    // if we reached here then we have a sync!
    assert(!isWaitSync());
    d = buildObject(fType);
    // waiting for the next object no matter the previous was successfull or not...
    setWaitSync();
}

inline TypeBase* 
Transport::buildObject(UnitType fType)
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

////////////////////////////////////////////////////////
// Writing methods                                    //
////////////////////////////////////////////////////////
inline void
Transport::write(uint64_t intd)
{
    fixEndianness64(&intd);
    write(&intd, sizeof(uint64_t));
}

inline void
Transport::write(int64_t intd)
{
    fixEndianness64(&intd);
    write(&intd, sizeof(int64_t));
}

inline void
Transport::write(uint32_t intd)
{
    fixEndianness32(&intd);
    write(&intd, sizeof(uint32_t));
}

inline void
Transport::write(int32_t intd)
{
    fixEndianness32(&intd);
    write(&intd, sizeof(int32_t));
}

inline void
Transport::write(uint16_t intd)
{
    fixEndianness16(&intd);
    write(&intd, sizeof(uint16_t));
}

inline void
Transport::write(int16_t intd)
{
    fixEndianness16(&intd);
    write(&intd, sizeof(int16_t));
}

inline void
Transport::write(uint8_t intd)
{
    write(&intd, sizeof(uint8_t));
}

inline void
Transport::write(int8_t intd)
{
    write(&intd, sizeof(int8_t));
}


inline void
Transport::write(float floatd)
{
    fixEndianness32(&floatd);
    write(&floatd, sizeof(float));
}

inline void
Transport::write(double doubled)
{
    fixEndianness64(&doubled);
    write(&doubled, sizeof(double));
}

inline void
Transport::write(const std::string& stringd)
{
    uint32_t stringd_len = stringd.length();
    writeChecksumed(stringd_len);
    write(stringd.c_str(), stringd_len);
}


////////////////////////////////////////////////////////
// Reading methods                                    //
////////////////////////////////////////////////////////
inline void
Transport::read(uint64_t& intd)
{
    read(&intd, sizeof(uint64_t));
    fixEndianness64(&intd);
}

inline void
Transport::read(int64_t& intd)
{
    read(&intd, sizeof(int64_t));
    fixEndianness64(&intd);
}

inline void
Transport::read(uint32_t& intd)
{
    read(&intd, sizeof(uint32_t));
    fixEndianness32(&intd);
}

inline void
Transport::read(int32_t& intd)
{
    read(&intd, sizeof(int32_t));
    fixEndianness32(&intd);
}


inline void
Transport::read(uint16_t& intd)
{
    read(&intd, sizeof(uint16_t));
    fixEndianness16(&intd);
}

inline void
Transport::read(int16_t& intd)
{
    read(&intd, sizeof(int16_t));
    fixEndianness16(&intd);
}

inline void
Transport::read(uint8_t& intd)
{
    read(&intd, sizeof(uint8_t));
}

inline void
Transport::read(int8_t& intd)
{
    read(&intd, sizeof(int8_t));
}

inline void
Transport::read(float& floatd)
{
    read(&floatd, sizeof(float));
    fixEndianness32(&floatd);
}

inline void
Transport::read(double& doubled)
{
    read(&doubled, sizeof(double));
    fixEndianness64(&doubled);
}

inline void
Transport::read(std::string& stringd)
{
    uint32_t stringd_len;
    readChecksumed(stringd_len);
    if(isWaitSync()) {
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
inline void
Transport::write(const void* ptr, uint32_t size)
{
    uint8_t* bptr = (uint8_t*)ptr;
    for(uint32_t i = 0; i < size; ++i) {
        if(!mDevice->write(bptr+i, 1)) {
            mState = DEVICE_ERROR;
            break;
        }
        mWriteChecksum += *(bptr+i);
    }
}

inline void
Transport::read(void* ptr, uint32_t size) 
{
    uint8_t* bptr = (uint8_t*)ptr;
    for(uint32_t i = 0; i < size; ++i) {
        if(!mDevice->read(bptr+i, 1)) {
            mState = DEVICE_ERROR;
            break;
        }
        mReadChecksum += *(bptr+i);
    }
}

template <class T>
void
Transport::readChecksumed(T& data) 
{
    // save the current checksum
    ChecksumType curChecksum = mReadChecksum;
    // reset it
    mReadChecksum = 0;
    // read the data
    read(data);
    // read and get the calculated chechsums
    ChecksumType computedChecksum = mReadChecksum;
    ChecksumType readChecksum;
    read(readChecksum);
    // restore the saved checksum
    mReadChecksum += curChecksum;
    // check the checksum and change the device state if needed 
    if(readChecksum != computedChecksum) {
        setWaitSync();
    }
}

template <class T>
void
Transport::writeChecksumed(const T& data) 
{
    // save the current checksum
    ChecksumType curChecksum = mWriteChecksum;
    // reset it
    mWriteChecksum = 0;
    // write the data
    write(data);
    // write the checksum
    write(mWriteChecksum);
    // restore the saved checksum
    mWriteChecksum += curChecksum;
}


////////////////////////////////////////////////////////
// Endianness                                         //
////////////////////////////////////////////////////////
template <ConfigEndianness E, int L> 
inline void
Transport::fixEndianness(void* ptr)
{
}

template <> 
inline void
Transport::fixEndianness<ENDIAN_SWAP, 2>(void* ptr)
{
    uint16_t& v = *(uint16_t*)ptr;
    v = (v>>8) | 
        (v<<8);
}

template <> 
inline void
Transport::fixEndianness<ENDIAN_SWAP, 4>(void* ptr)
{
    uint32_t& v = *(uint32_t*)ptr;
    v = (v>>24) | 
        ((v<<8) & 0x00FF0000) |
        ((v>>8) & 0x0000FF00) |
        (v<<24);
}

template <> 
inline void
Transport::fixEndianness<ENDIAN_SWAP, 8>(void* ptr)
{
    uint64_t& v = *(uint64_t*)ptr;
    v = (v>>56) | 
        ((v<<40) & 0x00FF000000000000) |
        ((v<<24) & 0x0000FF0000000000) |
        ((v<<8)  & 0x000000FF00000000) |
        ((v>>8)  & 0x00000000FF000000) |
        ((v>>24) & 0x0000000000FF0000) |
        ((v>>40) & 0x000000000000FF00) |
        (v<<56);
}

template <typename C>
void 
ConfiguredTransport<C>::fixEndianness16(void* ptr)
{
    fixEndianness<C::Endianness,2>(ptr);
}

template <typename C>
void 
ConfiguredTransport<C>::fixEndianness32(void* ptr)
{
    fixEndianness<C::Endianness,4>(ptr);
}

template <typename C>
void 
ConfiguredTransport<C>::fixEndianness64(void* ptr)
{
    fixEndianness<C::Endianness,8>(ptr);
}

} // namespace ygg

#endif //YGG_TRANSPORT_IMPL_HPP
