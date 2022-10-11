#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

if [[ -z ${BSH_BUILD_PLATFORM-} ]]; then
	>&2 echo "BSH_BUILD_PLATFORM not set"
	exit 1
fi
if [[ -z ${BSH_HOST_ARCH-} ]]; then
	>&2 echo "BSH_HOST_ARCH not set"
	exit 1
fi
if [[ -z ${BSH_HOST_PLATFORM-} ]]; then
	>&2 echo "BSH_HOST_PLATFORM not set"
	exit 1
fi
if [[ -z ${BSH_HOST_LIBC-} ]]; then
	>&2 echo "BSH_HOST_LIBC not set"
	exit 1
fi
if [[ -z ${BSH_STATIC_DYNAMIC-} ]]; then
	>&2 echo "BSH_STATIC_DYNAMIC not set"
	exit 1
fi
if [[ -z ${BSH_DEBUG_RELEASE-} ]]; then
	>&2 echo "BSH_DEBUG_RELEASE not set"
	exit 1
fi
if [[ -z ${RELEASE_NAME-} ]]; then
	>&2 echo "RELEASE_NAME not set"
	exit 1
fi
if [[ -z ${RELEASE_TYPE-} ]]; then
	>&2 echo "RELEASE_TYPE not set"
	exit 1
fi
if [[ -z ${MOD_ID-} ]]; then
	>&2 echo "MOD_ID not set"
	exit 1
fi
if [[ -z ${ASSET_PATH-} ]]; then
	>&2 echo "ASSET_PATH not set"
	exit 1
fi
if [[ -z ${SEPARATE_DEBUG-} ]]; then
	>&2 echo "SEPARATE_DEBUG not set"
	exit 1
fi
if [[ -z ${DEBUG_ASSET_PATH-} ]]; then
	>&2 echo "DEBUG_ASSET_PATH not set"
	exit 1
fi

case $BSH_HOST_ARCH-$BSH_HOST_PLATFORM-$BSH_HOST_LIBC-$BSH_STATIC_DYNAMIC in
x86_64-linux-gnu-static) ;;
x86_64-linux-gnu-dynamic) ;;
x86_64-windows-mingw-static) ;;
x86_64-windows-mingw-dynamic) ;;
x86_64-windows-msvc-static) ;;
x86_64-windows-msvc-dynamic) ;;
x86-windows-msvc-static) ;;
x86-windows-msvc-dynamic) ;;
x86_64-darwin-macos-static) ;;
x86_64-darwin-macos-dynamic) ;;
aarch64-darwin-macos-static) ;;
aarch64-darwin-macos-dynamic) ;;
x86-android-bionic-static) ;;
x86_64-android-bionic-static) ;;
arm-android-bionic-static) ;;
aarch64-android-bionic-static) ;;
*) >&2 echo "configuration $BSH_HOST_ARCH-$BSH_HOST_PLATFORM-$BSH_HOST_LIBC-$BSH_STATIC_DYNAMIC is not supported" && exit 1;;
esac

if [[ $BSH_HOST_PLATFORM == android ]]; then
	android_platform=android-30
	if [[ -z "${JAVA_HOME_8_X64-}" ]]; then
		>&2 echo "JAVA_HOME_8_X64 not set"
		exit 1
	fi
	if [[ -z "${ANDROID_SDK_ROOT-}" ]]; then
		>&2 echo "ANDROID_SDK_ROOT not set"
		exit 1
	fi
	if [[ -z "${ANDROID_NDK_LATEST_HOME-}" ]]; then
		>&2 echo "ANDROID_NDK_LATEST_HOME not set"
		exit 1
	fi
fi

if [[ $BSH_HOST_PLATFORM-$BSH_HOST_LIBC == windows-msvc ]]; then
	case $BSH_HOST_ARCH in
	x86_64) vs_env_arch=x64;;
	x86)    vs_env_arch=x86;;
	esac
	. ./.github/vs-env.sh $vs_env_arch
elif [[ $BSH_HOST_PLATFORM == darwin ]]; then
	# may need export SDKROOT=$(xcrun --show-sdk-path --sdk macosx11.1)
	CC=clang
	CXX=clang++
	if [[ $BSH_HOST_ARCH == aarch64 ]]; then
		if [[ $BSH_STATIC_DYNAMIC == static ]]; then
			export MACOSX_DEPLOYMENT_TARGET=11.0
		fi
		CC+=" -arch arm64"
		CXX+=" -arch arm64"
	else
		if [[ $BSH_STATIC_DYNAMIC == static ]]; then
			export MACOSX_DEPLOYMENT_TARGET=10.9
		fi
		CC+=" -arch x86_64"
		CXX+=" -arch x86_64"
	fi
	export CC
	export CXX
elif [[ $BSH_HOST_PLATFORM == android ]]; then
	case $BSH_HOST_ARCH in
	x86_64)  android_toolchain_prefix=x86_64-linux-android    ; android_system_version=21; android_arch_abi=x86_64     ;;
	x86)     android_toolchain_prefix=i686-linux-android      ; android_system_version=19; android_arch_abi=x86        ;;
	aarch64) android_toolchain_prefix=aarch64-linux-android   ; android_system_version=21; android_arch_abi=arm64-v8a  ;;
	arm)     android_toolchain_prefix=armv7a-linux-androideabi; android_system_version=19; android_arch_abi=armeabi-v7a;;
	esac
	android_toolchain_dir=$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64
	CC=$android_toolchain_dir/bin/$android_toolchain_prefix$android_system_version-clang
	CXX=$android_toolchain_dir/bin/$android_toolchain_prefix$android_system_version-clang++
	LD=$android_toolchain_dir/bin/$android_toolchain_prefix-ld
	AR=$android_toolchain_dir/bin/llvm-ar
	echo $AR
	CC+=" -fPIC"
	CXX+=" -fPIC"
	LD+=" -fPIC"
	export CC
	export CXX
	export LD
	export AR
else
	export CC=gcc
	export CXX=g++
fi

if [[ -d build ]]; then
	rm -r build
fi

c_args=
c_link_args=
if [[ $BSH_HOST_PLATFORM-$BSH_HOST_LIBC != windows-msvc ]]; then
	c_args+=\'-ffunction-sections\',
	c_args+=\'-fdata-sections\',
	if [[ $BSH_HOST_PLATFORM == darwin ]]; then
		c_link_args+=\'-Wl,-dead_strip\',
	else
		c_link_args+=\'-Wl,--gc-sections\',
	fi
fi
if [[ $BSH_HOST_PLATFORM-$BSH_STATIC_DYNAMIC == darwin-static ]]; then
	if [[ $BSH_HOST_ARCH == aarch64 ]]; then
		c_args+=\'-mmacosx-version-min=11.0\',
		c_link_args+=\'-mmacosx-version-min=11.0\',
	else
		c_args+=\'-mmacosx-version-min=10.9\',
		c_link_args+=\'-mmacosx-version-min=10.9\',
	fi
fi

meson_configure=meson
if [[ $BSH_DEBUG_RELEASE == release ]]; then
	meson_configure+=$'\t'-Dbuildtype=debugoptimized
fi
meson_configure+=$'\t'-Db_strip=false
meson_configure+=$'\t'-Db_staticpic=false
meson_configure+=$'\t'-Dinstall_check=true
meson_configure+=$'\t'-Dmod_id=$MOD_ID
if [[ $BSH_HOST_ARCH-$BSH_HOST_PLATFORM-$BSH_HOST_LIBC-$BSH_STATIC_DYNAMIC == x86_64-linux-gnu-static ]]; then
	meson_configure+=$'\t'-Dbuild_render=true
	meson_configure+=$'\t'-Dbuild_font=true
fi
if [[ $BSH_STATIC_DYNAMIC == static ]]; then
	meson_configure+=$'\t'-Dstatic=prebuilt
	if [[ $BSH_HOST_PLATFORM == windows ]]; then
		if [[ $BSH_HOST_LIBC == msvc ]]; then
			meson_configure+=$'\t'-Db_vscrt=static_from_buildtype
		else
			c_link_args+=\'-static\',
			c_link_args+=\'-static-libgcc\',
			c_link_args+=\'-static-libstdc++\',
		fi
	elif [[ $BSH_HOST_PLATFORM == linux ]]; then
		c_link_args+=\'-static-libgcc\',
		c_link_args+=\'-static-libstdc++\',
	fi
else
	if [[ $BSH_BUILD_PLATFORM == linux ]]; then
		meson_configure+=$'\t'-Dworkaround_elusive_bzip2=true
	fi
	if [[ $BSH_BUILD_PLATFORM == darwin ]]; then
		meson_configure+=$'\t'-Dworkaround_elusive_bzip2=true
		meson_configure+=$'\t'-Dworkaround_elusive_bzip2_lib_dir=/usr/local/opt/bzip2/lib
		meson_configure+=$'\t'-Dworkaround_elusive_bzip2_include_dir=/usr/local/opt/bzip2/include
		meson_configure+=$'\t'-Dworkaround_elusive_bzip2_static=true
	fi
fi
if [[ $BSH_HOST_PLATFORM == linux ]] && [[ $BSH_HOST_ARCH != aarch64 ]]; then
	# certain file managers can't run PIEs https://bugzilla.gnome.org/show_bug.cgi?id=737849
	meson_configure+=$'\t'-Db_pie=false
	c_link_args+=\'-no-pie\',
fi
stable_or_beta=no
if [[ $RELEASE_TYPE == beta ]]; then
	meson_configure+=$'\t'-Dbeta=true
	stable_or_beta=yes
fi
if [[ $RELEASE_TYPE == stable ]]; then
	stable_or_beta=yes
fi
if [[ $RELEASE_TYPE == snapshot ]]; then
	meson_configure+=$'\t'-Dsnapshot=true
	meson_configure+=$'\t'-Dsnapshot_id=$(echo $RELEASE_NAME | cut -d '-' -f 2) # $RELEASE_NAME is snapshot-X
fi
if [[ $RELEASE_TYPE == snapshot ]] && [[ $MOD_ID != 0 ]]; then
	>&2 echo "mods and snapshots do not mix"
	exit 1
fi
if [[ $stable_or_beta == yes ]] && [[ $MOD_ID != 0 ]]; then
	# mods and snapshots both check their snapshot_id against whatever version starcatcher.us/TPT has
	meson_configure+=$'\t'-Dsnapshot_id=$(echo $RELEASE_NAME | cut -d '.' -f 3) # $RELEASE_NAME is vX.Y.Z
fi
if [[ $RELEASE_TYPE == snapshot ]] || [[ $MOD_ID != 0 ]]; then
	meson_configure+=$'\t'-Dupdate_server=starcatcher.us/TPT
fi
if [[ $RELEASE_TYPE != dev ]]; then
	meson_configure+=$'\t'-Dignore_updates=false
fi
if [[ $BSH_HOST_PLATFORM-$BSH_HOST_LIBC == windows-mingw ]]; then
	if [[ $BSH_BUILD_PLATFORM == linux ]]; then
		meson_configure+=$'\t'--cross-file=.github/mingw-ghactions.ini
	fi
else
	# LTO simply doesn't work with MinGW. I have no idea why and I also don't care.
	meson_configure+=$'\t'-Db_lto=true
fi
if [[ $BSH_HOST_PLATFORM-$BSH_HOST_ARCH == darwin-aarch64 ]]; then
	meson_configure+=$'\t'--cross-file=.github/macaa64-ghactions.ini
fi
if [[ $RELEASE_TYPE == tptlibsdev ]] && ([[ $BSH_HOST_PLATFORM == windows ]] || [[ $BSH_STATIC_DYNAMIC == static ]]); then
	if [[ -z "${GITHUB_REPOSITORY_OWNER-}" ]]; then
		>&2 echo "GITHUB_REPOSITORY_OWNER not set"
		exit 1
	fi
	if [[ "$BSH_HOST_ARCH-$BSH_HOST_PLATFORM-$BSH_HOST_LIBC-$BSH_STATIC_DYNAMIC $BSH_BUILD_PLATFORM" == "x86_64-windows-mingw-dynamic linux" ]]; then
		>&2 echo "this configuration is not supported in tptlibsdev mode"
		touch $ASSET_PATH
		exit 0
	fi
	tptlibsbranch=$(echo $RELEASE_NAME | cut -d '-' -f 2-) # $RELEASE_NAME is tptlibsdev-BRANCH
	if [[ ! -d build-tpt-libs/tpt-libs ]]; then
		mkdir -p build-tpt-libs
		cd build-tpt-libs
		git clone https://github.com/$GITHUB_REPOSITORY_OWNER/tpt-libs --branch $tptlibsbranch --depth 1
		cd ..
	fi
	tpt_libs_vtag=v00000000000000
	if [[ ! -f build-tpt-libs/tpt-libs/.ok ]]; then
		cd build-tpt-libs/tpt-libs
		BSH_VTAG=$tpt_libs_vtag ./build.sh
		touch .ok
		cd ../../subprojects
		for i in tpt-libs-prebuilt-*; do
			if [[ -d $i ]]; then
				rm -r $i
			fi
		done
		7z x ../build-tpt-libs/tpt-libs/temp/libraries.zip
		cd ..
	fi
	meson_configure+=$'\t'-Dtpt_libs_vtag=$tpt_libs_vtag
fi
if [[ $BSH_HOST_PLATFORM == android ]]; then
	android_platform=android-30
	meson_configure+=$'\t'--cross-file=android/cross/$BSH_HOST_ARCH.ini
	cat << ANDROID_INI > .github/android-ghactions.ini
[constants]
andriod_ndk_toolchain_bin = '$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin'
andriod_sdk_build_tools = '$ANDROID_SDK_ROOT/build-tools/32.0.0'

[properties]
# android_ndk_toolchain_prefix comes from the correct cross-file in ./android/cross
android_ndk_toolchain_prefix = android_ndk_toolchain_prefix
android_platform = '$android_platform'
android_platform_jar = '$ANDROID_SDK_ROOT/platforms/' + android_platform + '/android.jar'
java_runtime_jar = '$JAVA_HOME_8_X64/jre/lib/rt.jar'

[binaries]
# android_ndk_toolchain_prefix comes from the correct cross-file in ./android/cross
c = andriod_ndk_toolchain_bin / (android_ndk_toolchain_prefix + 'clang')
cpp = andriod_ndk_toolchain_bin / (android_ndk_toolchain_prefix + 'clang++')
strip = andriod_ndk_toolchain_bin / 'llvm-strip'
javac = '$JAVA_HOME_8_X64/bin/javac'
jar = '$JAVA_HOME_8_X64/bin/jar'
d8 = andriod_sdk_build_tools / 'd8'
aapt = andriod_sdk_build_tools / 'aapt'
aapt2 = andriod_sdk_build_tools / 'aapt2'
zipalign = andriod_sdk_build_tools / 'zipalign'
apksigner = andriod_sdk_build_tools / 'apksigner'
ANDROID_INI
	meson_configure+=$'\t'--cross-file=.github/android-ghactions.ini
	meson_configure+=$'\t'-Dhttp=false
fi
meson_configure+=$'\t'-Dc_args=[$c_args]
meson_configure+=$'\t'-Dcpp_args=[$c_args]
meson_configure+=$'\t'-Dc_link_args=[$c_link_args]
meson_configure+=$'\t'-Dcpp_link_args=[$c_link_args]
$meson_configure build
cd build
if [[ $BSH_BUILD_PLATFORM == windows ]]; then
	set +e
	ninja -v -d keeprsp
	ninja_code=$?
	set -e
	cat powder.exe.rsp
	[[ $ninja_code == 0 ]];
else
	ninja -v
fi
strip=strip
objcopy=objcopy
strip_target=$ASSET_PATH
if [[ $BSH_HOST_PLATFORM == android ]]; then
	strip=$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-$strip
	objcopy=$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-$objcopy
	strip_target=libpowder.so
fi
if [[ $SEPARATE_DEBUG == yes ]] && [[ $BSH_HOST_PLATFORM-$BSH_HOST_LIBC != windows-msvc ]]; then
	$objcopy --only-keep-debug $strip_target $DEBUG_ASSET_PATH
	$strip --strip-debug --strip-unneeded $strip_target
	$objcopy --add-gnu-debuglink $DEBUG_ASSET_PATH $strip_target
fi
if [[ $BSH_HOST_PLATFORM == android ]]; then
	$JAVA_HOME_8_X64/bin/keytool -genkeypair -keystore keystore.jks -alias androidkey -validity 10000 -keyalg RSA -keysize 2048 -keypass bagelsbagels -storepass bagelsbagels -dname "CN=nobody"
	meson configure -Dandroid_keystore=$(realpath keystore.jks)
	ANDROID_KEYSTORE_PASS=bagelsbagels ninja android/powder.apk
	mv android/powder.apk powder.apk
fi
