#ifndef YGG_DATA_TYPE_REGISTRY
#define YGG_DATA_TYPE_REGISTRY

#include "yggTypes.hpp"
#include "yggConfig.hpp"
#include <list>
#include <vector>
#include <cassert>

namespace ygg
{

class TypeRegistry 
{
    template <typename MT, typename MI, typename MC> friend class Manager;
public:
    class ManifestData : public TypeRegistrator<ManifestData>
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
        void write(DeviceBase& dev) const;
        void read(DeviceBase& dev);
        DescriptorList mDescriptorRecords;
    };
    class SystemCmdData: public TypeRegistrator<SystemCmdData>
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
            void write(DeviceBase& dev) const;
            void read(DeviceBase& dev);
            bool operator==(const Type& type);
        private:
            bool isValidCommandType(uint32_t cmdType);
        private:
            Type mType;
    };
    // keeps the pair of descriptor and it's enabled/disabled status
    struct DescriptorState 
    {
        DescriptorState(TypeDescriptorBase* desc = NULL, bool en = false);
        TypeDescriptorBase* descriptor;
        bool enabled;
    };
    typedef std::vector<DescriptorState>        TypeDescriptorArray;
    typedef typename TypeDescriptorArray::const_iterator TypeDescriptorConstIt;
    typedef TypeBase::UnitType        UnitType;
    typedef TypeBase::VersionType     VersionType;
    typedef std::vector<UnitType>     TypeIdMap;
    
    TypeRegistry();
    ~TypeRegistry();

//public:
    // public API
    template<class Type> bool addType(const std::string& name, const int version);
    TypeBase* instantiateForeignType(UnitType fType) const;
    TypeBase* instantiateOwnType(UnitType oType) const;
    bool      isOwnTypeEnabled(UnitType oType) const;
    bool      isForeignTypeEnabled(UnitType oType) const;
    void      initialize();
    TypeBase* extractManifest();
    UnitType  findTypeId(const std::string& name, const VersionType version);
    void      applyManifest(ManifestData* md);
    void      acceptType(UnitType oType, UnitType fType);
    bool      isManifestReceved() const;
    void      setManifestReceived(bool flag);

    TypeDescriptorConstIt descriptorBegin() const;
    TypeDescriptorConstIt descriptorEnd() const;

private:
    UnitType  foreignTypeToOwnType(const UnitType fType) const;
    void      setTypeState(uint32_t typeId, bool enable);
    const     DescriptorState& descriptorStateAt(uint32_t typeId) const;
    DescriptorState& descriptorStateAt(uint32_t typeId);
    bool      isValidType(uint32_t typeId) const;
    void      reset();

private:
    TypeIdMap           mTypeMap;
    TypeDescriptorArray mDescriptors;
    bool                mManifestReceived;
    const UnitType      INVALID_TYPE_ID;
};


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
TypeRegistry::ManifestData::write(DeviceBase& dev) const 
{
    dev.write_checksumed((uint32_t)mDescriptorRecords.size());
    DescriptorListConstIt dit = mDescriptorRecords.begin();
    DescriptorListConstIt edit = mDescriptorRecords.end();
    for(; dit != edit; ++dit) {
        dev.write(dit->mId);
        dev.write(dit->mVersion);
        dev.write(dit->mName);
    }
}

inline void 
TypeRegistry::ManifestData::read(DeviceBase& dev) 
{
    uint32_t dSize;
    dev.read_checksumed(dSize);
    if(dev.isReadable()) {
        for(uint32_t i = 0; i < dSize; ++i) {
            mDescriptorRecords.push_back(DescriptorRecord());
            DescriptorRecord& drecord = mDescriptorRecords.back();
            dev.read(drecord.mId);
            dev.read(drecord.mVersion);
            dev.read(drecord.mName);
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
TypeRegistry::SystemCmdData::write(DeviceBase& dev) const
{
    dev.write((uint32_t)mType);
}

inline void 
TypeRegistry::SystemCmdData::read(DeviceBase& dev)  
{
    uint32_t cmdType;
    dev.read(cmdType);
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

inline
TypeRegistry::TypeRegistry() 
 :  mManifestReceived(false),
    INVALID_TYPE_ID(std::numeric_limits<UnitType>::max())
{}

inline
TypeRegistry::~TypeRegistry()
{
    reset();
}

template<class Type>
inline bool 
TypeRegistry::addType(const std::string& name, const int version)
{
    if(mDescriptors.size() == INVALID_TYPE_ID) {
        return false;
    }
    // if the manifest and command data types are not registered 
    // yet reserve enough space for them and go ahead
    if(mDescriptors.size() < 2) {
        mDescriptors.resize(2);
    }
    TypeDescriptorBase* tDesc = new TypeDescriptor<Type>();
    TypeDescriptor<Type>::sId = mDescriptors.size();
    TypeDescriptor<Type>::sVersion = version;
    TypeDescriptor<Type>::sName = name;
    mDescriptors.push_back(DescriptorState(tDesc, false));
    return true;
}

inline TypeBase*  
TypeRegistry::instantiateForeignType(UnitType fType) const
{
    UnitType oType = foreignTypeToOwnType(fType);
    if(oType != INVALID_TYPE_ID) {
        return instantiateOwnType(oType);
    }
    return NULL;
}

inline TypeBase* 
TypeRegistry::instantiateOwnType(UnitType oType) const
{
    if(isValidType(oType) && isOwnTypeEnabled(oType)) {
        const DescriptorState& state = descriptorStateAt(oType);
        return state.descriptor->create();
    }
    return NULL;
}

inline bool  
TypeRegistry::isForeignTypeEnabled(UnitType fType) const
{
    UnitType oType = foreignTypeToOwnType(fType);
    return isOwnTypeEnabled(oType);
}

inline bool 
TypeRegistry::isOwnTypeEnabled(UnitType oType) const
{
    if(isValidType(oType)) {
        return descriptorStateAt(oType).enabled;
    }
    return false;
}

inline void 
TypeRegistry::initialize()
{
    mDescriptors.resize(std::max(mDescriptors.size(),(size_t)2));

    // hard-register ManifestData
    TypeDescriptorBase* mDesc = new TypeDescriptor<ManifestData>();
    TypeDescriptor<ManifestData>::sId = 0;
    TypeDescriptor<ManifestData>::sVersion = 0;
    TypeDescriptor<ManifestData>::sName = "ManifestData";
    mDescriptors[0] = DescriptorState(mDesc);
    acceptType(0, 0);
    // hard-register SystemCmdData
    TypeDescriptorBase* cDesc = new TypeDescriptor<SystemCmdData>();
    TypeDescriptor<SystemCmdData>::sId = 1;
    TypeDescriptor<SystemCmdData>::sVersion = 0;
    TypeDescriptor<SystemCmdData>::sName = "SystemCmdData";
    mDescriptors[1] = DescriptorState(cDesc);
    acceptType(1, 1);
}

inline TypeBase* 
TypeRegistry::extractManifest()
{
    ManifestData* d = new ManifestData();
    TypeDescriptorConstIt dit = mDescriptors.begin();
    TypeDescriptorConstIt edit = mDescriptors.end();
    for(;  dit != edit; ++dit) {
        d->addRecord(dit->descriptor);
    }
    return d;
}

inline typename TypeRegistry::UnitType 
TypeRegistry::findTypeId(const std::string& name, const VersionType version)
{
    TypeDescriptorConstIt dit = mDescriptors.begin();
    TypeDescriptorConstIt edit = mDescriptors.end();
    for(;  dit != edit; ++dit) {
        if(dit->descriptor->typeName() == name &&
           dit->descriptor->typeVersion() == version) {
            return dit->descriptor->typeId();
        }
    }
    return INVALID_TYPE_ID;
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
        if(oType != INVALID_TYPE_ID)  { 
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
    if(fType >= mTypeMap.size()) {
        mTypeMap.resize(fType+1, INVALID_TYPE_ID);
    }
    mTypeMap[fType] = oType;
    setTypeState(oType, true);    
}

inline bool 
TypeRegistry::isManifestReceved() const 
{
    return mManifestReceived;
}

inline void 
TypeRegistry::setManifestReceived(bool flag)
{
    mManifestReceived = flag;
}

inline TypeRegistry::TypeDescriptorConstIt
TypeRegistry::descriptorBegin() const
{
    return mDescriptors.begin();
}

inline TypeRegistry::TypeDescriptorConstIt
TypeRegistry::descriptorEnd() const
{
    return mDescriptors.end();
}

inline typename TypeRegistry::UnitType 
TypeRegistry::foreignTypeToOwnType(const UnitType fType) const
{
    return (fType < mTypeMap.size()) ? mTypeMap[fType]
                                     : INVALID_TYPE_ID;
}

inline void 
TypeRegistry::setTypeState(uint32_t typeId, bool enable) 
{
    if(isValidType(typeId)) {
        descriptorStateAt(typeId).enabled = enable;
    }
}

inline const typename TypeRegistry::DescriptorState& 
TypeRegistry::descriptorStateAt(uint32_t typeId) const     
{
    assert(isValidType(typeId));
    return mDescriptors[typeId];
}

inline typename TypeRegistry::DescriptorState& 
TypeRegistry::descriptorStateAt(uint32_t typeId) 
{
    assert(isValidType(typeId));
    return mDescriptors[typeId];
}

inline bool 
TypeRegistry::isValidType(uint32_t typeId) const
{
    return typeId < mDescriptors.size();
}

inline void 
TypeRegistry::reset()
{
    TypeDescriptorConstIt dit = mDescriptors.begin();
    TypeDescriptorConstIt edit = mDescriptors.end();
    for(;  dit != edit; ++dit) {
        delete dit->descriptor;
    }
    mDescriptors.clear();
    mTypeMap.clear();
    setManifestReceived(false);
}


} // namespace ygg

#endif //YGG_DATA_TYPE_REGISTRY

