#ifndef KEYCONFIG_H_
#define KEYCONFIG_H_

#include <vector>
#include <tuple>
#include <string>

using KeyconfigItem = std::tuple<
	ByteString, // context
	bool, // ctrl
	bool, // shift
	bool, // alt
	int, // scancode
	ByteString // function
>;
using Keyconfig = std::vector<KeyconfigItem>;

#endif // KEYCONFIG_H_
