#ifndef YGG_DEVICE_BASE_HPP
#define YGG_DEVICE_BASE_HPP

#include "yggBaseTypes.hpp"
#include <string>

namespace ygg 
{

class DeviceBase
{
public:
    virtual void close() = 0;
    virtual bool read(void* ptr, uint32_t size) = 0;
    virtual bool write(const void* ptr, uint32_t size) = 0;
    virtual bool isOpen() = 0;
}; 


} // namespace ygg

#endif //YGG_DEVICE_BASE_HPP
