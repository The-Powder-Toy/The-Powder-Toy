#pragma once

// We don't want to define our own assert but we also can't trust the outside world to NOT define NDEBUG, so we undefine it ourselves.
// TODO: Undo this hack once the outside world stops being untrustworthy.

#ifdef NDEBUG
# undef NDEBUG
#endif

#include <cassert>
