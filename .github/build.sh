#!/bin/bash

set -euo pipefail
IFS=$'\n\t'

if [ -z "${PLATFORM_SHORT-}" ]; then
	>&2 echo "PLATFORM_SHORT not set"
	exit 1
fi

if [ -z "${STATIC_DYNAMIC-}" ]; then
	>&2 echo "STATIC_DYNAMIC not set"
	exit 1
fi

if [ -z "${RELTYPECFG-}" ]; then
	>&2 echo "RELTYPECFG not set"
	exit 1
fi

if [ -z "${build_sh_init-}" ]; then
	if [ $PLATFORM_SHORT == "win" ]; then
		for i in C:/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio/**/**/VC/Auxiliary/Build/vcvarsall.bat; do
			vcvarsall_path=$i
		done
		cat << BUILD_INIT_BAT > .github/build_init.bat
@echo off
call "${vcvarsall_path}" x64
bash -c 'build_sh_init=1 ./.github/build.sh'
BUILD_INIT_BAT
		./.github/build_init.bat
	else
		build_sh_init=1 ./.github/build.sh
	fi
	exit 0
fi

other_flags=
bin_suffix=
static_flag=
if [ $STATIC_DYNAMIC == "static" ]; then
	static_flag=-Dstatic=prebuilt
	if [ $PLATFORM_SHORT == "win" ]; then
		other_flags+=$'\t-Db_vscrt=static_from_buildtype'
	fi
fi
if [ $PLATFORM_SHORT == "lin" ]; then
	# We use gcc on lin; sadly, gcc + lto + libstdc++ + pthread = undefined reference to
	# pthread_create, thanks to weak symbols in libstdc++.so (or something). See
	# https://gcc.gnu.org/legacy-ml/gcc-help/2017-03/msg00081.html
	other_flags+=$'\t-Db_asneeded=false\t-Dcpp_link_args=-Wl,--no-as-needed'
	if [ $STATIC_DYNAMIC == "static" ]; then
		other_flags+=$'\t-Dbuild_render=true\t-Dbuild_font=true'
	fi
fi
if [ $PLATFORM_SHORT == "win" ]; then
	bin_suffix=$bin_suffix.exe
fi
if echo $RELTYPECFG | base64 -d | grep snapshot; then
	other_flags+=$'\t-Dupdate_server=starcatcher.us/TPT'
fi
meson -Dbuildtype=release -Db_pie=false -Db_staticpic=false -Db_lto=true $static_flag -Dinstall_check=true $other_flags `echo $RELTYPECFG | base64 -d` build
cd build
ninja
if [ $PLATFORM_SHORT == "lin" ] || [ $PLATFORM_SHORT == "mac" ]; then
	strip powder$bin_suffix
fi
cp powder$bin_suffix ..
