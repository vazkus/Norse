#ifndef HRE_SERIALIZATIN_HPP
#define HRE_SERIALIZATIN_HPP

// chibios includes
#include "ch.hpp"
#include "hal.h"
#include "test.h"
#include "evtimer.h"
// yggdrasil
#include "yggSerializationManager.hpp"
#include "yggChibiosTraits.hpp"
#include "yggConfig.hpp"

struct ChibiosConfig
{
    // configuration of the serialization system
    const static ygg::ConfigCommunication   Serialization    = ygg::COMMUNICATION_NONBLOCKING;
    const static ygg::ConfigCommunication   Deserialization  = ygg::COMMUNICATION_NONBLOCKING;
    const static ygg::ConfigEndianness      Endianness       = ygg::ENDIAN_NATIVE;
    const static ygg::ConfigManifest        ManifestRequired = ygg::MANIFEST_REQUIRED;
    // various parameters of the serialization system
    const static int BasePriority = NORMALPRIO+10;
    const static int InputQueueSize = 10;
    const static int OutputQueueSize = 10;
    const static int ManifestRequestMs = 1000;
};
class ChInputHandler;

typedef ygg::SerializationManager<
                                   ygg::ChibiosSystemTraits, 
                                   ChInputHandler, 
                                   ygg::DummyTransport, 
                                   ChibiosConfig
                                 > sm;

#endif //HRE_SERIALIZATIN_HPP
