cplusplus {{
#include <vector>
typedef std::vector<uint8_t> DataVector;
}}

cplusplus {{
    #include "inet/linklayer/base/MACFrameBase_m.h"
}}

namespace dsme;

cplusplus {{
inline std::ostream& operator<<(std::ostream& out, const unsigned char& ch)
{ 
   out << (uint16_t)ch;
   return out;
}
}}

class noncobject DataVector;

class ::inet::MACFrameBase;

packet DSMEFrame extends ::inet::MACFrameBase
{
    DataVector data;
}
