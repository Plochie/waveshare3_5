#pragma once

#include <cstddef>
#include "apps/app_common.h"

namespace app_registry {

// Returns the static table of registered apps.
const app_descriptor_t *all(size_t &count);

} // namespace app_registry
