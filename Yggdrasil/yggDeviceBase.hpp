#ifndef YGG_DEVICE_BASE_HPP
#define YGG_DEVICE_BASE_HPP

#include "yggBaseTypes.hpp"
#include <string>

namespace ygg 
{

class DeviceBase
{
public:
    typedef uint8_t  ChecksumType;

public:
    virtual void write(uint32_t intd) = 0;
    virtual void write(int32_t intd) = 0;
    virtual void write(uint16_t intd) = 0;
    virtual void write(int16_t intd) = 0;
    virtual void write(uint8_t intd) = 0;
    virtual void write(int8_t intd) = 0;
    virtual void write(float floatd) = 0;
    virtual void write(double doubled) = 0;
    virtual void write(const std::string& stringd) = 0;

    virtual void read(uint32_t& intd) = 0;
    virtual void read(int32_t& intd) = 0;
    virtual void read(uint16_t& intd) = 0;
    virtual void read(int16_t& intd) = 0;
    virtual void read(uint8_t& intd) = 0;
    virtual void read(int8_t& intd) = 0;
    virtual void read(float& floatd) = 0;
    virtual void read(double& doubled) = 0;
    virtual void read(std::string& stringd) = 0;

    virtual bool isReadable() const = 0;
    virtual void setReadable() = 0;
    virtual bool isError() const = 0;
    virtual void setError() = 0;
    virtual bool isWaitSynch() const = 0;
    virtual void setWaitSynch() = 0;
    virtual bool isWaitManifest() const = 0;
    virtual void setWaitManifest() = 0;

    template <class T> void read_checksumed(T& td);
    template <class T> void write_checksumed(const T& td);
protected:
    ChecksumType  mReadChecksum;
    ChecksumType  mWriteChecksum;
};

template <class T>
void
DeviceBase::read_checksumed(T& data) 
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
        setWaitSynch();
    }
}

template <class T>
void
DeviceBase::write_checksumed(const T& data) 
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

} // namespace ygg

#endif //YGG_DEVICE_BASE_HPP
