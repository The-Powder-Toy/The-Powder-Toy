#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

curl "https://starcatcher.us/TPT/perform-release.lua?mod=$MOD_ID&type=$RELEASE_TYPE&name=$RELEASE_NAME&commit=$GITHUB_SHA" > perform_release
if ! grep "Release complete" < perform_release > /dev/null; then
	cat perform_release
	exit 1
fi
