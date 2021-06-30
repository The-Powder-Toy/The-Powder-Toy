#!/bin/bash

set -euo pipefail
IFS=$'\n\t'

if [ -z "${PLATFORM_SHORT-}" ]; then
	>&2 echo "PLATFORM_SHORT not set (lin, mac, win)"
	exit 1
fi
if [ -z "${MACHINE_SHORT-}" ]; then
	>&2 echo "MACHINE_SHORT not set (x86_64, i686)"
	exit 1
fi
if [ -z "${TOOLSET_SHORT-}" ]; then
	>&2 echo "TOOLSET_SHORT not set (gcc, clang, mingw)"
	exit 1
fi
if [ -z "${STATIC_DYNAMIC-}" ]; then
	>&2 echo "STATIC_DYNAMIC not set (static, dynamic)"
	exit 1
fi
if [ -z "${RELNAME-}" ]; then
	>&2 echo "RELNAME not set"
	exit 1
fi
if [ -z "${RELTYPE-}" ]; then
	>&2 echo "RELTYPE not set"
	exit 1
fi
if [ -z "${MOD_ID-}" ]; then
	>&2 echo "MOD_ID not set"
	exit 1
fi

if [ -z "${build_sh_init-}" ]; then
	if [ $PLATFORM_SHORT == "win" ]; then
		for i in C:/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio/**/**/VC/Auxiliary/Build/vcvarsall.bat; do
			vcvarsall_path=$i
		done
		if [ $MACHINE_SHORT == "x86_64" ]; then
			x64_x86=x64
		else
			x64_x86=x86
		fi
		cat << BUILD_INIT_BAT > .github/build_init.bat
@echo off
call "${vcvarsall_path}" ${x64_x86}
bash -c 'build_sh_init=1 ./.github/build.sh'
BUILD_INIT_BAT
		./.github/build_init.bat
	else
		build_sh_init=1 ./.github/build.sh
	fi
	exit 0
fi

other_flags=$'\t-Dmod_id='
other_flags+=$MOD_ID
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
stable_or_beta="n"
if [ "$RELTYPE" == "beta" ]; then
	other_flags+=$'\t-Dbeta=true'
	stable_or_beta="y"
fi
if [ "$RELTYPE" == "stable" ]; then
	stable_or_beta="y"
fi
if [ "$RELTYPE" == "snapshot" ]; then
	other_flags+=$'\t-Dsnapshot=true\t-Dsnapshot_id='
	other_flags+=`echo $RELNAME | cut -d '-' -f 2` # $RELNAME is snapshot-X
fi
if [ "$RELTYPE" == "snapshot" ] && [ "$MOD_ID" != "0" ]; then
	>&2 echo "mods and snapshots do not mix"
	exit 1
fi
if [ "$stable_or_beta" == "y" ] && [ "$MOD_ID" != "0" ]; then
	# mods and snapshots both check their snapshot_id against whatever version starcatcher.us/TPT has
	other_flags+=$'\t-Dsnapshot_id='
	other_flags+=`echo $RELNAME | cut -d '.' -f 3` # $RELNAME is vX.Y.Z
fi
if [ "$RELTYPE" == "snapshot" ] || [ "$MOD_ID" != "0" ]; then
	other_flags+=$'\t-Dupdate_server=starcatcher.us/TPT'
fi
if [ "$RELTYPE" != "dev" ]; then
	other_flags+=$'\t-Dignore_updates=false'
fi
meson -Dbuildtype=release -Db_pie=false -Db_staticpic=false -Db_lto=true $static_flag -Dinstall_check=true $other_flags build
cd build
ninja
if [ $PLATFORM_SHORT == "lin" ] || [ $PLATFORM_SHORT == "mac" ]; then
	strip powder$bin_suffix
fi
cp powder$bin_suffix ..
