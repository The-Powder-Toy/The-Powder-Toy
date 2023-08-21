#include "Client.h"
#include "prefs/GlobalPrefs.h"
#include <emscripten.h>
#include <iostream>

void Client::LoadAuthUser()
{
	ByteString newUsername, newSessionKey;
	if (EM_ASM_INT({
		return (document.querySelector("#PowderSessionInfo [name='Username']") &&
		        document.querySelector("#PowderSessionInfo [name='SessionKey']")) ? 1 : 0;
	}))
	{
		newUsername = ByteString(std::unique_ptr<char, decltype(&free)>((char *)EM_ASM_PTR({
			return stringToNewUTF8(document.querySelector("#PowderSessionInfo [name='Username']").value);
		}), free).get());
		newSessionKey = ByteString(std::unique_ptr<char, decltype(&free)>((char *)EM_ASM_PTR({
			return stringToNewUTF8(document.querySelector("#PowderSessionInfo [name='SessionKey']").value);
		}), free).get());
	}
	else
	{
		std::cerr << "required #PowderSessionInfo elements not found, can't authenticate" << std::endl;
	}
	if (newUsername.size() && newSessionKey.size())
	{
		authUser.UserID = -1; // Not quite valid but evaluates to true and that's all that matters for this codebase.
		authUser.Username = newUsername;
		authUser.SessionID = "(invalid)";
		authUser.SessionKey = newSessionKey;
		authUser.UserElevation = User::ElevationNone; // We don't deal with this in the browser.
	}
	else
	{
		authUser.UserID = 0;
	}
}

void Client::SaveAuthUser()
{
	// Nothing is; the cookie headers in the login and logout responses take care of state management.
}
