#ifndef YGG_DATA_TYPES_HPP
#define YGG_DATA_TYPES_HPP

#include "yggDeviceBase.hpp"
#include <string>
#include <limits>

namespace ygg
{

class TypeRegistry;
class DeviceBase;
class TypeBase;

//////////////////////////////////////////////////////
// type descriptors                                 // 
//////////////////////////////////////////////////////
class TypeDescriptorBase
{
public:
    typedef uint8_t UnitType;
    typedef uint8_t VersionType;
protected:
    TypeDescriptorBase()
    {}
public:
    virtual TypeBase*   create() const = 0;
    virtual UnitType    typeId() const = 0;
    virtual VersionType typeVersion() const = 0;
    virtual const std::string& typeName() const = 0;
private:
};

template <class Type>
class TypeDescriptor : public TypeDescriptorBase
{
    friend class TypeRegistry;
    TypeDescriptor()
    {
    }
public:
    TypeBase* create() const 
    {
        return new Type();
    }
    static UnitType id() 
    {
        return sId;
    }
    virtual UnitType    typeId() const 
    {
        return TypeDescriptor<Type>::sId;
    }
    virtual VersionType typeVersion() const
    {
        return TypeDescriptor<Type>::sVersion;
    }
    virtual const std::string& typeName() const
    {
        return TypeDescriptor<Type>::sName;
    }
private:
    static UnitType    sId;
    static VersionType sVersion;
    static std::string sName;
};

template <class T> typename TypeDescriptor<T>::UnitType TypeDescriptor<T>::sId;
template <class T> typename TypeDescriptor<T>::VersionType TypeDescriptor<T>::sVersion;
template <class T> std::string TypeDescriptor<T>::sName;


//////////////////////////////////////////////////////
// data types                                       //
//////////////////////////////////////////////////////
class TypeBase
{
public:
    typedef TypeDescriptorBase::UnitType    UnitType;
    typedef TypeDescriptorBase::VersionType VersionType;
    virtual ~TypeBase()
    {}
    virtual void write(DeviceBase& dev) const = 0;
    virtual void read(DeviceBase& dev) = 0;
    virtual UnitType id() const = 0;

};

template<typename Type>
class TypeRegistrator : public TypeBase
{
    friend class TypeRegistry;
public:
    virtual UnitType id()  const
    {
        return TypeDescriptor<Type>::id();
    }
};

} // namespace ygg

#endif //YGG_DATA_TYPES_HPP
