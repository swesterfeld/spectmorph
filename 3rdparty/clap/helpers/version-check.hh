#pragma once

/*
 * Check the clap version against the supported versions for this version of CLAP Helpers.
 * It defines two clap versions, a min and max version, and static asserts that the CLAP
 * we are using is in range.
 *
 * Workflow wise, if you are about to make clap-helpers incompatible with a version, set
 * the prior version to max, commit and push, and then move the min to current and max to 2 0 0
 * again.
 */

#include "clap/version.h"

#if CLAP_VERSION_LT(1,2,0)
static_assert(false, "Clap version must be at least 1.2.0")
#endif

#if CLAP_VERSION_GE(2,0,0)
   static_assert(false, "Clap version must be at most 1.x")
#endif