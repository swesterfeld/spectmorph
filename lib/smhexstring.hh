// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_HEX_STRING_HH
#define SPECTMORPH_HEX_STRING_HH

#include <vector>
#include <string>

namespace SpectMorph
{

namespace HexString
{

bool        decode (const std::string& str, std::vector<unsigned char>& out);
std::string encode (const std::vector<unsigned char>& data);

}

}

#endif
