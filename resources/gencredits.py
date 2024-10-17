import urllib.error
import urllib.request
import json

def get_url(url : str) -> bytes | None:
	try:
		req = urllib.request.Request(url)
		data = urllib.request.urlopen(req, timeout=10)
		page = data.read()

		return page
	except urllib.error.URLError as e:
		print(f"{url} - {e}")
		return None

def fetch_gh_contributors() -> list[any]:
	page = 1
	ret = []
	while True:
		data = get_url(f"https://api.github.com/repos/The-Powder-Toy/The-Powder-Toy/contributors?page={page}")
		contributors = json.loads(data)
		if not len(contributors):
			break
		ret.extend(contributors)
		page = page + 1

	return ret

def get_github_json(github_contributors : list[any]) -> list[str]:
	ret = []
	for contributor in github_contributors:
		ret.append(contributor["login"])

	return ret

def get_orig_json() -> list[dict[str, str | int]]:
	"""Credits that appeared in intro text in older versions"""

	return [
		{ "realname" : "Stanislaw K Skowronek", "message" : "Designed the original Powder Toy"},
		{ "realname" : "Simon Robertshaw",      "message" : "Wrote the website, current server owner"},
		{ "realname" : "Skresanov Savely",      "message" : ""},
		{ "realname" : "Pilihp64",              "message" : ""},
		{ "realname" : "Catelite",              "message" : ""},
		{ "realname" : "Victoria Hoyle",        "message" : ""},
		{ "realname" : "Nathan Cousins",        "message" : ""},
		{ "realname" : "jacksonmj",             "message" : ""},
		{ "realname" : "Felix Wallin",          "message" : ""},
		{ "realname" : "Lieuwe Mosch",          "message" : ""},
		{ "realname" : "Anthony Boot",          "message" : ""},
		{ "realname" : "Me4502",                "message" : ""},
		{ "realname" : "MaksProg",              "message" : ""},
		{ "realname" : "jacob1",                "message" : ""},
		{ "realname" : "mniip",                 "message" : ""},
		{ "realname" : "LBPHacker",             "message" : ""},
	]

def get_moderator_json() -> list[dict[str, str | int]]:
	"""Current and former moderators"""

	return [
		{ "username" : "jacob1",      "role" : "Moderator" },
		{ "username" : "LBPHacker",   "role" : "Moderator" },
		{ "username" : "Sylvi",       "role" : "Moderator" },
		{ "username" : "CCl2F2",      "role" : "Moderator" },
		{ "username" : "catsoften",   "role" : "Moderator" },
		{ "username" : "Denderth",    "role" : "Moderator" },
		{ "username" : "Simon",       "role" : "Moderator" },
		{ "username" : "Mrprocom",    "role" : "Moderator" },
		{ "username" : "jacksonmj",   "role" : "Former Staff" },
		{ "username" : "Pilihp64",    "role" : "Former Staff" },
		{ "username" : "Catelite",    "role" : "Former Staff" },
		{ "username" : "boxmein",     "role" : "Former Staff" },
		{ "username" : "lolzy",       "role" : "Former Staff" },
		{ "username" : "Xenocide",    "role" : "Former Staff" },
		{ "username" : "triclops200", "role" : "Former Staff" },
		{ "username" : "devast8a",    "role" : "Former Staff" },
		{ "username" : "HK6",         "role" : "Former Staff" },
		{ "username" : "FrankBro",    "role" : "Former Staff" },
		{ "username" : "doxin",       "role" : "Former Staff" },
		{ "username" : "ief015",      "role" : "Former Staff" },
		{ "username" : "ad",          "role" : "Former Staff" },
	]

def process() -> any:
	github_contributors = fetch_gh_contributors()
	github = get_github_json(github_contributors)

	orig = get_orig_json()
	mods = get_moderator_json()

	data = {
		"GitHub" : github,
		"OrigCredits" : orig,
		"Moderators" : mods,
	}

	with open("credits.json", "w") as f:
		json.dump(data, f)

process()
