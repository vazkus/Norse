#ifndef RAT_BASIC_TYPES_HPP
#define RAT_BASIC_TYPES_HPP

#include <stdint.h>

namespace rat 
{

struct Axes 
{ 
    Axes(uint8_t xp = 0, uint8_t yp = 0, uint8_t zp = 0) 
     : x(xp),y(yp),z(zp)
    {}
    uint8_t x, y, z;
};

} // namespace rat

#endif // RAT_BASIC_TYPES_HPP
