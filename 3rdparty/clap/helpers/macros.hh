#pragma once

#if defined(__cplusplus) && __cplusplus >= 202002L
#   define CLAP_HELPERS_UNLIKELY [[unlikely]]
#else
#   define CLAP_HELPERS_UNLIKELY
#endif
