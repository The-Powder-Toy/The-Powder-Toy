#!/bin/bash

set -euo pipefail
IFS=$'\n\t'

if [ -z "${PLATFORM_SHORT-}" ]; then
	>&2 echo "PLATFORM_SHORT not set (lin, mac, win, and)"
	exit 1
fi
if [ -z "${MACHINE_SHORT-}" ]; then
	>&2 echo "MACHINE_SHORT not set (x86_64, i686, arm64, arm)"
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
	if [ $TOOLSET_SHORT == "msvc" ]; then
		for i in C:/Program\ Files*/Microsoft\ Visual\ Studio/**/**/VC/Auxiliary/Build/vcvarsall.bat; do
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

if [ -d build ]; then
	rm -r build
fi

other_flags=$'\t-Dmod_id='
other_flags+=$MOD_ID
bin_suffix=
bin_prefix=
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
	if [ $STATIC_DYNAMIC == "static" ] && [ $TOOLSET_SHORT == "gcc" ]; then
		other_flags+=$'\t-Dbuild_render=true\t-Dbuild_font=true'
	fi
fi
if [ $TOOLSET_SHORT == "mingw" ]; then
	bin_suffix=$bin_suffix.exe
fi
if [ $PLATFORM_SHORT == "and" ]; then
	bin_suffix=$bin_suffix.apk
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
lto_flag=-Db_lto=true
if [ $TOOLSET_SHORT == "mingw" ]; then
	# This simply doesn't work with MinGW. I have no idea why and I also don't care.
	lto_flag=
	if [ $PLATFORM_SHORT == "lin" ]; then
		other_flags+=$'\t--cross-file=.github/mingw-ghactions.ini'
	fi
fi
if [ $PLATFORM_SHORT == "mac" ]; then
	macosx_version_min=10.9
	if [ $MACHINE_SHORT == "arm64" ]; then
		macosx_version_min=10.15
		other_flags+=$'\t--cross-file=.github/macaa64-ghactions.ini'
	fi
	export CFLAGS=-mmacosx-version-min=$macosx_version_min
	export CXXFLAGS=-mmacosx-version-min=$macosx_version_min
	export LDFLAGS=-mmacosx-version-min=$macosx_version_min
fi
powder_bin=${bin_prefix}powder$bin_suffix
if [ "$RELTYPE" == "tptlibsdev" ]; then
	if [ -z "${GITHUB_REPOSITORY_OWNER-}" ]; then
		>&2 echo "GITHUB_REPOSITORY_OWNER not set (whose tpt-libs to clone?)"
		exit 1
	fi
	tptlibsbranch=`echo $RELNAME | cut -d '-' -f 2-` # $RELNAME is tptlibsdev-BRANCH
	if [ ! -d tpt-libs ]; then
		git clone https://github.com/$GITHUB_REPOSITORY_OWNER/tpt-libs --branch $tptlibsbranch
	fi
	cd tpt-libs
	quad=$MACHINE_SHORT-$PLATFORM_SHORT-$TOOLSET_SHORT-$STATIC_DYNAMIC
	if [ ! -d patches/$quad ]; then
		cd ..
		echo "no prebuilt libraries for this configuration" > $powder_bin
		exit 0
	fi
	tpt_libs_vtag=v00000000000000
	if [ ! -f .ok ]; then
		VTAG=$tpt_libs_vtag ./build.sh
		touch .ok
		cd ../subprojects
		if [ -d tpt-libs-prebuilt-$quad-$tpt_libs_vtag ]; then
			rm -r tpt-libs-prebuilt-$quad-$tpt_libs_vtag
		fi
		7z x ../tpt-libs/temp/libraries.zip
	fi
	cd ..
	other_flags+=$'\t-Dtpt_libs_vtag='
	other_flags+=$tpt_libs_vtag
fi
if [ $PLATFORM_SHORT == "and" ]; then
	other_flags+=$'\t--cross-file='
	if [ $MACHINE_SHORT == "x86_64" ]; then
		other_flags+=android/cross/x86_64.ini
	fi
	if [ $MACHINE_SHORT == "i686" ]; then
		other_flags+=android/cross/x86.ini
	fi
	if [ $MACHINE_SHORT == "arm64" ]; then
		other_flags+=android/cross/arm64-v8a.ini
	fi
	if [ $MACHINE_SHORT == "arm" ]; then
		other_flags+=android/cross/armeabi-v7a.ini
	fi
	other_flags+=$'\t--cross-file=.github/android-ghactions.ini'
	if [ -z "${JAVA_HOME_8_X64-}" ]; then
		>&2 echo "JAVA_HOME_8_X64 not set (where is jdk?)"
		exit 1
	fi
	cat << BUILD_INIT_BAT > .github/jdk.ini
[properties]
java_runtime_jar = '$JAVA_HOME_8_X64/jre/lib/rt.jar'

[binaries]
javac = '$JAVA_HOME_8_X64/bin/javac'
jar = '$JAVA_HOME_8_X64/bin/jar'
BUILD_INIT_BAT
	other_flags+=$'\t--cross-file=.github/jdk.ini'
	other_flags+=$'\t-Dhttp=false'
fi
meson -Dbuildtype=release -Db_pie=false -Dworkaround_gcc_no_pie=true -Db_staticpic=false $lto_flag $static_flag -Dinstall_check=true $other_flags build
cd build
ninja
if [ $PLATFORM_SHORT == "and" ]; then
	$ANDROID_SDK_ROOT/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip libpowder.so
elif [ $PLATFORM_SHORT != "win" ]; then
	strip $powder_bin
fi
if [ $PLATFORM_SHORT == "and" ]; then
	$JAVA_HOME_8_X64/bin/keytool -genkeypair -keystore keystore.jks -alias androidkey -validity 10000 -keyalg RSA -keysize 2048 -keypass bagelsbagels -storepass bagelsbagels -dname "CN=nobody"
	meson configure -Dandroid_keystore=`readlink -f keystore.jks`
	ANDROID_KEYSTORE_PASS=bagelsbagels ninja powder.apk
fi
cp $powder_bin ..
