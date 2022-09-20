#!/usr/bin/env bash

set -euo pipefail
shopt -s globstar
IFS=$'\n\t'

if [[ -z ${1-} ]] || [[ -z ${2-} ]] || [[ -z ${3-} ]]; then
	>&2 echo "usage: ./update-wraps.sh OWNER REPO TAG"
	exit 1
fi

owner=$1
repo=$2
tag=$3

set +e
rm *.wrap > /dev/null 2>&1
set -e
for wrap in $(curl -sL "https://api.github.com/repos/$owner/$repo/releases/tags/$tag" | jq -r '.assets[].browser_download_url | select(endswith("wrap"))'); do
	echo $wrap
	curl -sLO $wrap
done

sed -i "s|tpt_libs_vtag = '[^']*'|tpt_libs_vtag = '$tag'|g" ../meson.build
