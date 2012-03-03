#ifndef HRE_SERIALIZATIN_HPP
#define HRE_SERIALIZATIN_HPP

// chibios includes
#include "ch.hpp"
#include "hal.h"
#include "test.h"
#include "evtimer.h"
// yggdrasil
#include "yggManager.hpp"
#include "yggChibiosTraits.hpp"
#include "yggConfig.hpp"

struct ChibiosConfig
{
    // configuration of the serialization system
    const static ygg::ConfigCommunication   Serialization    = ygg::COMMUNICATION_BLOCKING;
    const static ygg::ConfigCommunication   Deserialization  = ygg::COMMUNICATION_BLOCKING;
    const static ygg::ConfigEndianness      Endianness       = ygg::ENDIAN_IGNORE;
    const static ygg::ConfigSynchronization Synchronization  = ygg::SYNC_AUTO;
    const static ygg::ConfigManifest        ManifestRequired = ygg::MANIFEST_REQUIRED;
    // various parameters of the serialization system
    const static int BasePriority = NORMALPRIO+10;
    const static int InputQueueSize = 10;
    const static int OutputQueueSize = 10;
    const static int SyncAutoMs = 200;
    const static int ManifestRequestMs = 1000;
};
class ChInputHandler;

typedef ygg::Manager<ygg::ChibiosSystemTraits, ChInputHandler, ChibiosConfig> sm;

#endif //HRE_SERIALIZATIN_HPP
