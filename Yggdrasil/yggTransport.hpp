#ifndef YGG_TRANSPORT_HPP
#define YGG_TRANSPORT_HPP

#include "yggTypes.hpp"
#include "yggConfig.hpp"

namespace ygg
{

class DeviceBase;

class Transport 
{
    template <typename MT, typename MI, typename MC> friend class Manager;
    template <typename ST, typename SC> friend class Serializer;
    template <typename DT, typename DS, typename DI, typename CC> friend class Deserializer;
    friend class TypeRegistry;

protected:
    typedef TypeBase::UnitType  UnitType;
    typedef uint32_t            SyncType;
    typedef UnitType            ChecksumType;
    enum DeviceState 
    {
        DEVICE_READABLE,
        DEVICE_STOPPED,
        DEVICE_WAITING_SYNC,
        DEVICE_ERROR
    };
    enum 
    {
        SYNC_BYTE = 0xAB
    };

protected:
    Transport();
    void setTypeRegistry(TypeRegistry* registry);
    void start(DeviceBase* device);
    void stop();
    // status checking
    bool isReadable() const;
    void setReadable();
    bool isError() const;
    void setError();
    bool isStopped() const;
    void setStopped();
    bool isWaitSync() const;
    void setWaitSync();
    // writing serializable objects
    void writeData(const TypeBase* d);
    // reading serializable objects
    void readData(TypeBase*& d);
protected:
    UnitType  readObjectType();
    TypeBase* buildObject(UnitType fType);

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

    template <class T> void readChecksumed(T& td);
    template <class T> void writeChecksumed(const T& td);

protected:
    void write(const void* ptr, uint32_t size);
    void read(void* ptr, uint32_t size);

protected:
    DeviceBase*   mDevice;
    DeviceState   mState;
    TypeRegistry* mTypeRegistry;
    ChecksumType  mReadChecksum;
    ChecksumType  mWriteChecksum;
};

template <typename C>
class ConfiguredTransport : public Transport
{
protected:
    template <class T> void swap_endiannes(T& t) {}
    
};

} // namespace ygg

#endif //YGG_TRANSPORT_HPP
