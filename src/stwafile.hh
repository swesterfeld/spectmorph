#include "stwaudio.hh"
#include <string>

namespace STWAFile
{
    BseErrorType load (const std::string& filename, Stw::Codec::AudioHandle& audio_data);
    BseErrorType save (const std::string& filename, const Stw::Codec::Audio& audio_data);
};
