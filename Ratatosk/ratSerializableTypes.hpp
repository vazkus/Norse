#ifndef RAT_SERIALIZATION_TYPES_HPP
#define RAT_SERIALIZATION_TYPES_HPP

#include "ratTypes.hpp"
#include <list>
#include <cassert>

namespace rat
{
// a simple data type for serializing type descriptors...
template <typename T> class TypeRegistrator;

// a simple type that can be used for serialization of basic objects and object-arrays
template <typename Type, uint32_t size>
class BasicType : public ygg::TypeRegistrator<BasicType<Type, size> >
{
public:
    void write(ygg::DeviceBase& dev) const
    {
        for(uint32_t i = 0; i < size; ++i) {
            dev.write(m_array[i]);
        }
    }
    void read(ygg::DeviceBase& dev)  
    {
        for(uint32_t i = 0; i < size; ++i) {
            dev.read(m_array[i]);
        }
    }
private:
    Type  m_array[size];
};

// a simple type that can be used for serialization of string commands...
class StrCmdData: public ygg::TypeRegistrator<StrCmdData>
{
public:
    StrCmdData() 
    {}
    StrCmdData(const std::string& str)
     : mString(str)
    {}
    void write(ygg::DeviceBase& dev) const
    {
        dev.write(mString);
    }
    void read(ygg::DeviceBase& dev)  
    {
        dev.read(mString);
    }
    const std::string& string() const
    {
        return mString;
    }
private:
    std::string mString;
};

class LISData : public ygg::TypeRegistrator<LISData>
{
    typedef rat::Axes Axes;
public:
    LISData(const Axes& axes = Axes())
      : mAxes(axes)
    {
    }
    void write(ygg::DeviceBase& dev) const
    {
        dev.write(mAxes.x);
        dev.write(mAxes.y);
        dev.write(mAxes.z);
    }
    void read(ygg::DeviceBase& dev)  
    {
        dev.read(mAxes.x);
        dev.read(mAxes.y);
        dev.read(mAxes.z);
    }
    const Axes& axes() const
    {
        return mAxes;
    }
private:
    Axes mAxes;
};

class PingData: public ygg::TypeRegistrator<PingData>
{
public:
    PingData(const uint32_t& timeStamp = 0)
     : mTimeStamp(timeStamp)
    {
    }
    void write(ygg::DeviceBase& dev) const
    {
        dev.write(mTimeStamp);
    }
    void read(ygg::DeviceBase& dev)  
    {
        dev.read(mTimeStamp);
    }
    const uint32_t & timeStamp() const
    {
        return mTimeStamp;
    }
private:
    uint32_t mTimeStamp;
};

} // namespace rat

#endif //RAT_SERIALIZATION_TYPES_HPP
