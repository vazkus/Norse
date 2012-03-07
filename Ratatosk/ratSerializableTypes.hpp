#ifndef RAT_SERIALIZATION_TYPES_HPP
#define RAT_SERIALIZATION_TYPES_HPP

#include "ratTypes.hpp"
#include <list>
#include <cassert>

namespace rat
{
// a simple data type for serializing type descriptors...
template <typename T> class Serializable;

// a simple type that can be used for serialization of basic objects and object-arrays
template <typename Type, uint32_t size>
class BasicType : public ygg::Serializable<BasicType<Type, size> >
{
public:
    void write(ygg::Transport& transport) const
    {
        for(uint32_t i = 0; i < size; ++i) {
            transport.write(m_array[i]);
        }
    }
    void read(ygg::Transport& transport)  
    {
        for(uint32_t i = 0; i < size; ++i) {
            transport.read(m_array[i]);
        }
    }
private:
    Type  m_array[size];
};

// a simple type that can be used for serialization of string commands...
class StrCmdData: public ygg::Serializable<StrCmdData>
{
public:
    StrCmdData() 
    {}
    StrCmdData(const std::string& str)
     : mString(str)
    {}
    void write(ygg::Transport& transport) const
    {
        transport.write(mString);
    }
    void read(ygg::Transport& transport)  
    {
        transport.read(mString);
    }
    const std::string& string() const
    {
        return mString;
    }
private:
    std::string mString;
};

class LISData : public ygg::Serializable<LISData>
{
    typedef rat::Axes Axes;
public:
    LISData(const Axes& axes = Axes())
      : mAxes(axes)
    {
    }
    void write(ygg::Transport& transport) const
    {
        transport.write(mAxes.x);
        transport.write(mAxes.y);
        transport.write(mAxes.z);
    }
    void read(ygg::Transport& transport)  
    {
        transport.read(mAxes.x);
        transport.read(mAxes.y);
        transport.read(mAxes.z);
    }
    const Axes& axes() const
    {
        return mAxes;
    }
private:
    Axes mAxes;
};

class PingData: public ygg::Serializable<PingData>
{
public:
    PingData(const uint32_t& timeStamp = 0)
     : mTimeStamp(timeStamp)
    {
    }
    void write(ygg::Transport& transport) const
    {
        transport.write(mTimeStamp);
    }
    void read(ygg::Transport& transport)  
    {
        transport.read(mTimeStamp);
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
