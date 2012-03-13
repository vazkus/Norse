#ifndef YGG_TRANSPORT_HPP
#define YGG_TRANSPORT_HPP

#include "yggTypes.hpp"
#include "yggConfig.hpp"

namespace ygg
{

class Transport 
{
    template <typename T, typename S, typename I, typename L, typename C> friend class Deserializer;
protected:
    typedef TypeBase::UnitType  UnitType;
    typedef uint32_t            SyncType;
    typedef UnitType            ChecksumType;
    enum DeviceState 
    {
        DEVICE_FUNCTIONAL,
        DEVICE_STOPPED,
        DEVICE_WAITING_SYNC,
        DEVICE_ERROR
    };
    enum 
    {
        SYNC_BYTE = 0xAB
    };

public:
    // main API
    void write(uint64_t intd);
    void write(int64_t intd);
    void write(uint32_t intd);
    void write(int32_t intd);
    void write(uint16_t intd);
    void write(int16_t intd);
    void write(uint8_t intd);
    void write(int8_t intd);
    void write(float floatd);
    void write(double doubled);
    void write(const std::string& stringd);

    void read(uint64_t& intd);
    void read(int64_t& intd);
    void read(uint32_t& intd);
    void read(int32_t& intd);
    void read(uint16_t& intd);
    void read(int16_t& intd);
    void read(uint8_t& intd);
    void read(int8_t& intd);
    void read(float& floatd);
    void read(double& doubled);
    void read(std::string& stringd);

    template <class T> void readChecksumed(T& td);
    template <class T> void writeChecksumed(const T& td);

public:
    Transport();
    virtual void start() = 0;
    virtual void stop() = 0;
    // status checking
    bool isFunctional() const;
    void setFunctional();
    bool isError() const;
    void setError();
    bool isStopped() const;
    void setStopped();
    bool isWaitSync() const;
    void setWaitSync();

    // writing serializable objects
    void serialize(const TypeBase* d);
    // reading serializable objects
    void deserialize(TypeBase*& d);

protected:
    UnitType  readObjectType();
    TypeBase* buildObject(UnitType fType);
    template <ConfigEndianness E, int L> void fixEndianness(void* ptr);
    virtual void write(const void* ptr, uint32_t size) = 0;
    virtual void read(void* ptr, uint32_t size) = 0;

protected:
    virtual void fixEndianness16(void* ptr) = 0;
    virtual void fixEndianness32(void* ptr) = 0;
    virtual void fixEndianness64(void* ptr) = 0;
    virtual void swap(Transport& transport);

protected:
    DeviceState   mState;
    ChecksumType  mReadChecksum;
    ChecksumType  mWriteChecksum;
};



template <typename C, typename D>
class ConfiguredTransport : public Transport
{
public:
    ConfiguredTransport(D* device = NULL);
    virtual void start();
    virtual void stop();
    virtual void swap(ConfiguredTransport<C,D>& transport);
protected:
    virtual void fixEndianness16(void* ptr);
    virtual void fixEndianness32(void* ptr);
    virtual void fixEndianness64(void* ptr);
    virtual void write(const void* ptr, uint32_t size);
    virtual void read(void* ptr, uint32_t size);
protected:
    D* mDevice;
};


class DummyDevice
{};

template <typename C>
class ConfiguredTransport<C, DummyDevice>
{
public:
    void serialize(const TypeBase*)
    {}
    void deserialize(TypeBase*&)
    {}
    
};


} // namespace ygg

#endif //YGG_TRANSPORT_HPP
