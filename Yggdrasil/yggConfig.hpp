#ifndef YGG_CONFIGS_HPP
#define YGG_CONFIGS_HPP

namespace ygg {
    // Supported:
    //    All tested for both in/out, except mixed configs
    enum ConfigCommunication
    {
        COMMUNICATION_BLOCKING,
        COMMUNICATION_NONBLOCKING
    };
    // Supported:
    //    MANIFEST_REQUIRED: tested
    //    MANIFEST_IGNORE: not tested
    enum ConfigManifest
    {
        MANIFEST_REQUIRED,
        MANIFEST_IGNORE
    };
    // Not supported yet.
    enum ConfigEndPoint
    {
        ENDPOINT_SENDER,
        ENDPOINT_RECEIVER,
        ENDPOINT_SENDER_RECEIVER
    };
    // Not supported yet
    enum ConfigEndianness
    {
        ENDIAN_LITTLE,
        ENDIAN_BIG,
        ENDIAN_IGNORE
    };

} // namespace ygg

#endif //YGG_CONFIGS_HPP
