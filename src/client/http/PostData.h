#pragma once
#include "common/String.h"
#include <map>
#include <variant>

namespace http
{
	using StringData = ByteString;
	using FormData = std::map<ByteString, ByteString>;
	using PostData = std::variant<StringData, FormData>;
};
