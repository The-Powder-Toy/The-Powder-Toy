#include "Config.h"

#include "common/Format.h"
#include "common/Singleton.h"
#include "common/String.h"
#include "common/tpt-compat.h"
#include "common/tpt-inline.h"
#include "common/tpt-minmax.h"
#include "common/tpt-rand.h"
#include "common/tpt-rand.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cfloat>
#include <climits>
#include <cmath>
#include <condition_variable>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <ostream>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <fftw3.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef WIN
# include <sys/param.h>
# include <sys/poll.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/un.h>
#endif

#ifdef WIN
# include <shlobj.h>
# include <shlwapi.h>
# include <windows.h>
# include <winsock.h>
#endif

#include <curl/curl.h>
#include <bson/BSON.h>
#include <json/json-forwards.h>
#include <json/json.h>
#include <zlib.h>

#if !defined(FONTEDITOR) && !defined(RENDERER) && defined(LUACONSOLE)
# include "lua/LuaCompat.h"
#endif
#include "SDLCompat.h"
