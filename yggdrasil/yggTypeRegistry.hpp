#ifndef YGG_DATA_TYPE_REGISTRY
#define YGG_DATA_TYPE_REGISTRY

#include "yggTransport.hpp"
#include "yggTypes.hpp"
#include "yggConfig.hpp"
#include <list>
#include <vector>
#include <cassert>

namespace ygg
{

class TypeRegistry 
{
private:
    class ManifestData : public Serializable<ManifestData>
    {
        struct DescriptorRecord 
        {
            TypeDescriptorBase::UnitType    mId;
            TypeDescriptorBase::VersionType mVersion;
            std::string                     mName;
        };
    public:
        typedef std::list<DescriptorRecord>    DescriptorList;
        typedef typename DescriptorList::const_iterator DescriptorListConstIt;
        ManifestData();
        void addRecord(TypeDescriptorBase* desc);
        void write(Transport& transport) const;
        void read(Transport& transport);
        DescriptorList mDescriptorRecords;
    };
    class SystemCmdData: public Serializable<SystemCmdData>
    {
        public:
            enum Type 
            {   
                CMD_BEGIN,
                CMD_MANIFEST_REQUEST,
                CMD_END
            };
        public:
            SystemCmdData();
            SystemCmdData(const Type type);
            void write(Transport& transport) const;
            void read(Transport& transport);
            bool operator==(const Type& type);
        private:
            bool isValidCommandType(uint32_t cmdType);
        private:
            Type mType;
    };

public:
    // keeps the pair of descriptor and it's enabled/disabled status
    struct DescriptorState 
    {
        DescriptorState(TypeDescriptorBase* desc = NULL, bool en = false);
        TypeDescriptorBase* descriptor;
        bool enabled;
    };
    typedef std::vector<DescriptorState>        TypeDescriptorArray;
    typedef typename TypeDescriptorArray::const_iterator TypeDescriptorConstIt;

private:
    typedef TypeDescriptorBase::UnitType        UnitType;
    typedef TypeDescriptorBase::VersionType     VersionType;
    typedef std::vector<UnitType>               TypeIdMap;

public:
    // public API
    template<typename Type> static bool addType(const std::string& name, 
                                                const int version);
    template<typename Type> static bool isType(TypeBase* d);
    static TypeBase* instantiateForeignType(UnitType fType);
    static TypeBase* instantiateOwnType(UnitType oType);
    static bool      isOwnTypeEnabled(UnitType oType);
    static bool      isForeignTypeEnabled(UnitType oType);
    static void      initialize();
    static TypeBase* extractManifest();
    static UnitType  findTypeId(const std::string& name, const VersionType version);
    static void      applyManifest(ManifestData* md);
    static void      acceptType(UnitType oType, UnitType fType);
    static bool      isManifestReceved();
    static void      setManifestReceived(bool flag);

    static TypeDescriptorConstIt descriptorBegin();
    static TypeDescriptorConstIt descriptorEnd();

private:
    static UnitType  foreignTypeToOwnType(const UnitType fType);
    static void      setTypeState(uint32_t typeId, bool enable);
    static DescriptorState& descriptorStateAt(uint32_t typeId);
    static bool      isValidType(uint32_t typeId);
    static void      reset();

private:
    TypeRegistry();
    static TypeRegistry& self()
    {
        static TypeRegistry sTypeRegistry;
        return sTypeRegistry;
    }

private:
    TypeIdMap           mTypeMap;
    TypeDescriptorArray mDescriptors;
    bool                mManifestReceived;
    const UnitType      INVALID_TYPE_ID;
};


inline
TypeRegistry::TypeRegistry() 
 :  mManifestReceived(false),
    INVALID_TYPE_ID(std::numeric_limits<UnitType>::max())
{}


inline
TypeRegistry::ManifestData::ManifestData()
{}

inline void 
TypeRegistry::ManifestData::addRecord(TypeDescriptorBase* desc)
{
    assert(desc);
    mDescriptorRecords.push_back(DescriptorRecord());
    DescriptorRecord& drecord = mDescriptorRecords.back();
    drecord.mId = desc->typeId();
    drecord.mVersion = desc->typeVersion();
    drecord.mName = desc->typeName();
}

inline void 
TypeRegistry::ManifestData::write(Transport& transport) const
{
    transport.writeChecksumed((uint32_t)mDescriptorRecords.size());
    DescriptorListConstIt dit = mDescriptorRecords.begin();
    DescriptorListConstIt edit = mDescriptorRecords.end();
    for(; dit != edit; ++dit) {
        transport.write(dit->mId);
        transport.write(dit->mVersion);
        transport.write(dit->mName);
    }
}

inline void 
TypeRegistry::ManifestData::read(Transport& transport) 
{
    uint32_t dSize;
    transport.readChecksumed(dSize);
    if(transport.isFunctional()) {
        for(uint32_t i = 0; i < dSize; ++i) {
            mDescriptorRecords.push_back(DescriptorRecord());
            DescriptorRecord& drecord = mDescriptorRecords.back();
            transport.read(drecord.mId);
            transport.read(drecord.mVersion);
            transport.read(drecord.mName);
        }
    }
}

inline
TypeRegistry::SystemCmdData::SystemCmdData() 
{}

inline
TypeRegistry::SystemCmdData::SystemCmdData(const Type type)
    : mType(type)
{}

inline void 
TypeRegistry::SystemCmdData::write(Transport& transport) const
{
    transport.write((uint32_t)mType);
}

inline void 
TypeRegistry::SystemCmdData::read(Transport& transport)  
{
    uint32_t cmdType;
    transport.read(cmdType);
    if(isValidCommandType(cmdType)) {
        mType = (Type)cmdType;
    }
}

inline bool 
TypeRegistry::SystemCmdData::isValidCommandType(uint32_t cmdType)
{
    return cmdType > CMD_BEGIN && cmdType < CMD_END;
}

inline bool 
TypeRegistry::SystemCmdData::operator==(const Type& type) 
{
    return mType == type;
}

inline
TypeRegistry::DescriptorState::DescriptorState(TypeDescriptorBase* desc, bool en) 
    : descriptor(desc), enabled(en)
{}


template<class Type>
bool 
TypeRegistry::addType(const std::string& name, const int version)
{
    if(self().mDescriptors.size() == self().INVALID_TYPE_ID) {
        return false;
    }
    // if the manifest and command data types are not registered 
    // yet reserve enough space for them and go ahead
    if(self().mDescriptors.size() < 2) {
        self().mDescriptors.resize(2);
    }
    TypeDescriptorBase* tDesc = 
        new TypeDescriptor<Type>(self().mDescriptors.size(), version, name);
    self().mDescriptors.push_back(DescriptorState(tDesc, false));
    return true;
}

template<typename Type>
bool 
TypeRegistry::isType(TypeBase* d)
{
    return d->id() == TypeDescriptor<Type>::id();
}

inline TypeBase*  
TypeRegistry::instantiateForeignType(UnitType fType)
{
    UnitType oType = foreignTypeToOwnType(fType);
    if(oType != self().INVALID_TYPE_ID) {
        return instantiateOwnType(oType);
    }
    return NULL;
}

inline TypeBase* 
TypeRegistry::instantiateOwnType(UnitType oType) 
{
    if(isValidType(oType) && isOwnTypeEnabled(oType)) {
        const DescriptorState& state = descriptorStateAt(oType);
        return state.descriptor->create();
    }
    return NULL;
}

inline bool  
TypeRegistry::isForeignTypeEnabled(UnitType fType) 
{
    UnitType oType = foreignTypeToOwnType(fType);
    return isOwnTypeEnabled(oType);
}

inline bool 
TypeRegistry::isOwnTypeEnabled(UnitType oType) 
{
    if(isValidType(oType)) {
        return descriptorStateAt(oType).enabled;
    }
    return false;
}

inline void 
TypeRegistry::initialize()
{
    self().mDescriptors.resize(std::max(self().mDescriptors.size(),(size_t)2));

    // hard-register ManifestData
    TypeDescriptorBase* mDesc = new TypeDescriptor<ManifestData>(0,0,"ManifestData");
    self().mDescriptors[0] = DescriptorState(mDesc);
    acceptType(0, 0);
    // hard-register SystemCmdData
    TypeDescriptorBase* cDesc = new TypeDescriptor<SystemCmdData>(1,0,"SystemCmdData");
    self().mDescriptors[1] = DescriptorState(cDesc);
    acceptType(1, 1);
}

inline TypeBase* 
TypeRegistry::extractManifest()
{
    ManifestData* d = new ManifestData();
    TypeDescriptorConstIt dit = self().mDescriptors.begin();
    TypeDescriptorConstIt edit = self().mDescriptors.end();
    for(;  dit != edit; ++dit) {
        d->addRecord(dit->descriptor);
    }
    return d;
}

inline typename TypeRegistry::UnitType 
TypeRegistry::findTypeId(const std::string& name, const VersionType version)
{
    TypeDescriptorConstIt dit = self().mDescriptors.begin();
    TypeDescriptorConstIt edit = self().mDescriptors.end();
    for(;  dit != edit; ++dit) {
        if(dit->descriptor->typeName() == name &&
           dit->descriptor->typeVersion() == version) {
            return dit->descriptor->typeId();
        }
    }
    return self().INVALID_TYPE_ID;
}

inline void 
TypeRegistry::applyManifest(ManifestData* md)
{
    // iterate through the sent type descriptors and enables 
    // those that are present and match...
    typename ManifestData::DescriptorListConstIt dit = md->mDescriptorRecords.begin();
    typename ManifestData::DescriptorListConstIt edit = md->mDescriptorRecords.end();
    for(; dit != edit; ++dit) {
        // type is accepted if it has the same id AND version AND name!
        UnitType oType = findTypeId(dit->mName, dit->mVersion);
        if(oType != self().INVALID_TYPE_ID)  { 
            acceptType(oType, dit->mId);
        }
    }
    if(!md->mDescriptorRecords.empty()) {
        setManifestReceived(true);
    }
}

inline void 
TypeRegistry::acceptType(UnitType oType, UnitType fType) 
{
    if(fType >= self().mTypeMap.size()) {
        self().mTypeMap.resize(fType+1, self().INVALID_TYPE_ID);
    }
    self().mTypeMap[fType] = oType;
    setTypeState(oType, true);    
}

inline bool 
TypeRegistry::isManifestReceved() 
{
    return self().mManifestReceived;
}

inline void 
TypeRegistry::setManifestReceived(bool flag)
{
    self().mManifestReceived = flag;
}

inline TypeRegistry::TypeDescriptorConstIt
TypeRegistry::descriptorBegin() 
{
    return self().mDescriptors.begin();
}

inline TypeRegistry::TypeDescriptorConstIt
TypeRegistry::descriptorEnd() 
{
    return self().mDescriptors.end();
}

inline typename TypeRegistry::UnitType 
TypeRegistry::foreignTypeToOwnType(const UnitType fType) 
{
    return (fType < self().mTypeMap.size()) ? self().mTypeMap[fType]
                                            : self().INVALID_TYPE_ID;
}

inline void 
TypeRegistry::setTypeState(uint32_t typeId, bool enable) 
{
    if(isValidType(typeId)) {
        descriptorStateAt(typeId).enabled = enable;
    }
}

inline typename TypeRegistry::DescriptorState& 
TypeRegistry::descriptorStateAt(uint32_t typeId) 
{
    assert(isValidType(typeId));
    return self().mDescriptors[typeId];
}

inline bool 
TypeRegistry::isValidType(uint32_t typeId) 
{
    return typeId < self().mDescriptors.size();
}

inline void 
TypeRegistry::reset()
{
    TypeDescriptorConstIt dit = self().mDescriptors.begin();
    TypeDescriptorConstIt edit = self().mDescriptors.end();
    for(;  dit != edit; ++dit) {
        delete dit->descriptor;
    }
    self().mDescriptors.clear();
    self().mTypeMap.clear();
    setManifestReceived(false);
}

} // namespace ygg

#endif //YGG_DATA_TYPE_REGISTRY

