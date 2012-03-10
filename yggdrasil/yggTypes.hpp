#ifndef YGG_DATA_TYPES_HPP
#define YGG_DATA_TYPES_HPP

#include "yggBaseTypes.hpp"
#include <string>
#include <limits>

namespace ygg
{

class TypeRegistry;
class TypeBase;
class Transport;
class DummyType {};


class TypeBase
{
public:
    typedef uint8_t UnitType;
public:
    virtual ~TypeBase()
    {}
    virtual void write(Transport& out) const = 0;
    virtual void read(Transport& in)   = 0;
    virtual UnitType id() const = 0;
};

class TypeDescriptorBase
{
public:
    typedef TypeBase::UnitType UnitType;
    typedef uint8_t            VersionType;
public:
    virtual ~TypeDescriptorBase() 
    {}
    virtual UnitType           typeId() const = 0;
    virtual VersionType        typeVersion() const = 0;
    virtual const std::string& typeName() const = 0;
    virtual TypeBase* create() const = 0;
};

template <class Type>
class TypeDescriptor : public TypeDescriptorBase
{
private:
    friend class TypeRegistry;
    TypeDescriptor(UnitType id, VersionType version, const std::string& name) 
      : mVersion(version),
        mName(name)
    {
        sId = id;
    }
public:
    UnitType typeId() const
    {
        return sId;
    }
    VersionType typeVersion() const
    {
        return mVersion;
    }
    const std::string& typeName() const
    {
        return mName;
    }
    virtual TypeBase* create() const
    { 
        return new Type(); 
    }
public:
    static UnitType id()
    {
        return sId;
    }
private:
    VersionType mVersion;
    std::string mName;
    static UnitType sId;
};

template <class Type> TypeDescriptorBase::UnitType TypeDescriptor<Type>::sId;

template<typename Type>
class Serializable: public TypeBase
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
