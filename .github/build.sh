#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

if [[ -z ${BSH_BUILD_PLATFORM-} ]]; then >&2 echo "BSH_BUILD_PLATFORM not set"; exit 1; fi
if [[ -z      ${BSH_HOST_ARCH-} ]]; then >&2 echo      "BSH_HOST_ARCH not set"; exit 1; fi
if [[ -z  ${BSH_HOST_PLATFORM-} ]]; then >&2 echo  "BSH_HOST_PLATFORM not set"; exit 1; fi
if [[ -z      ${BSH_HOST_LIBC-} ]]; then >&2 echo      "BSH_HOST_LIBC not set"; exit 1; fi
if [[ -z ${BSH_STATIC_DYNAMIC-} ]]; then >&2 echo "BSH_STATIC_DYNAMIC not set"; exit 1; fi
if [[ -z  ${BSH_DEBUG_RELEASE-} ]]; then >&2 echo  "BSH_DEBUG_RELEASE not set"; exit 1; fi
if [[ -z       ${RELEASE_NAME-} ]]; then >&2 echo       "RELEASE_NAME not set"; exit 1; fi
if [[ -z       ${RELEASE_TYPE-} ]]; then >&2 echo       "RELEASE_TYPE not set"; exit 1; fi
if [[ -z             ${MOD_ID-} ]]; then >&2 echo             "MOD_ID not set"; exit 1; fi
if [[ -z     ${SEPARATE_DEBUG-} ]]; then >&2 echo     "SEPARATE_DEBUG not set"; exit 1; fi
if [[ -z       ${PACKAGE_MODE-} ]]; then >&2 echo       "PACKAGE_MODE not set"; exit 1; fi
if [[ -z         ${ASSET_PATH-} ]]; then >&2 echo         "ASSET_PATH not set"; exit 1; fi
if [[ -z   ${DEBUG_ASSET_PATH-} ]]; then >&2 echo   "DEBUG_ASSET_PATH not set"; exit 1; fi
if [[ -z           ${APP_NAME-} ]]; then >&2 echo           "APP_NAME not set"; exit 1; fi
if [[ -z        ${APP_COMMENT-} ]]; then >&2 echo        "APP_COMMENT not set"; exit 1; fi
if [[ -z            ${APP_EXE-} ]]; then >&2 echo            "APP_EXE not set"; exit 1; fi
if [[ -z             ${APP_ID-} ]]; then >&2 echo             "APP_ID not set"; exit 1; fi
if [[ -z           ${APP_DATA-} ]]; then >&2 echo           "APP_DATA not set"; exit 1; fi
if [[ -z         ${APP_VENDOR-} ]]; then >&2 echo         "APP_VENDOR not set"; exit 1; fi

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
wasm32-emscripten-emscripten-static) ;;
*) >&2 echo "configuration $BSH_HOST_ARCH-$BSH_HOST_PLATFORM-$BSH_HOST_LIBC-$BSH_STATIC_DYNAMIC is not supported" && exit 1;;
esac

if [[ $BSH_HOST_PLATFORM == android ]]; then
	android_platform=android-31
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

if [[ -z ${BSH_NO_PACKAGES-} ]]; then
	case $BSH_HOST_PLATFORM in
	android)
		(
			export PATH=$ANDROID_SDK_ROOT/cmdline-tools/latest/bin:$ANDROID_SDK_ROOT/tools/bin:$PATH
			sdkmanager "platforms;$android_platform"
		)
		;;
	windows)
		if [[ $BSH_BUILD_PLATFORM-$BSH_HOST_LIBC == windows-mingw ]]; then
			pacman -S --noconfirm --needed mingw-w64-ucrt-x86_64-gcc
			if [[ $BSH_STATIC_DYNAMIC == static ]]; then
				pacman -S --noconfirm --needed mingw-w64-ucrt-x86_64-{cmake,7zip,jq} patch
			else
				pacman -S --noconfirm --needed mingw-w64-ucrt-x86_64-{pkgconf,bzip2,luajit,jsoncpp,curl,SDL2,libpng,meson,fftw,jq}
			fi
			export PKG_CONFIG=$(which pkg-config.exe)
		fi
		;;
	linux)
		sudo apt update
		if [[ $BSH_STATIC_DYNAMIC == static ]]; then
			sudo apt install libc6-dev libc6-dev-i386
		else
			sudo apt install libluajit-5.1-dev libcurl4-openssl-dev libfftw3-dev zlib1g-dev libsdl2-dev libbz2-dev libjsoncpp-dev
		fi
		;;
	darwin)
		brew install pkg-config binutils
		if [[ $BSH_STATIC_DYNAMIC != static ]]; then
			brew install luajit curl fftw zlib sdl2 bzip2 jsoncpp
		fi
		;;
	emscripten)
		git clone https://github.com/emscripten-core/emsdk.git --branch 3.1.30
		cd emsdk
		./emsdk install latest
		./emsdk activate latest
		. ./emsdk_env.sh
		cd ..
		;;
	esac
fi

function inplace_sed() {
	local subst=$1
	local path=$2
	if [[ $BSH_BUILD_PLATFORM == darwin ]]; then
		sed -i "" -e $subst $path
	else
		sed -i $subst $path
	fi
}

if [[ $BSH_HOST_PLATFORM-$BSH_HOST_LIBC == windows-msvc ]]; then
	case $BSH_HOST_ARCH in
	x86_64) vs_env_arch=x64;;
	x86)    vs_env_arch=x86;;
	esac
	VS_ENV_PARAMS=$vs_env_arch$'\t'-vcvars_ver=14.1
	. ./.github/vs-env.sh
elif [[ $BSH_HOST_PLATFORM == darwin ]]; then
	# may need export SDKROOT=$(xcrun --show-sdk-path --sdk macosx11.1)
	CC=clang
	CXX=clang++
	if [[ $BSH_HOST_ARCH == aarch64 ]]; then
		macos_min_ver=11.0
		CC+=" -arch arm64"
		CXX+=" -arch arm64"
	else
		macos_min_ver=10.13
		CC+=" -arch x86_64"
		CXX+=" -arch x86_64"
	fi
	if [[ $BSH_STATIC_DYNAMIC == static ]]; then
		export MACOSX_DEPLOYMENT_TARGET=$macos_min_ver
	fi
	export CC
	export CXX
elif [[ $BSH_HOST_PLATFORM == android ]]; then
	case $BSH_HOST_ARCH in
	x86_64)  android_toolchain_prefix=x86_64-linux-android    ; android_system_version=21; android_arch_abi=x86_64     ;;
	x86)     android_toolchain_prefix=i686-linux-android      ; android_system_version=21; android_arch_abi=x86        ;;
	aarch64) android_toolchain_prefix=aarch64-linux-android   ; android_system_version=21; android_arch_abi=arm64-v8a  ;;
	arm)     android_toolchain_prefix=armv7a-linux-androideabi; android_system_version=21; android_arch_abi=armeabi-v7a;;
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
	c_args+=\'-mmacosx-version-min=$macos_min_ver\',
	c_link_args+=\'-mmacosx-version-min=$macos_min_ver\',
fi

meson_configure=meson$'\t'setup
if [[ $BSH_DEBUG_RELEASE == release ]]; then
	meson_configure+=$'\t'-Dbuildtype=debugoptimized
fi
if [[ $BSH_HOST_PLATFORM == darwin ]]; then
	meson_configure+=$'\t'-Dmanifest_macos_min_ver=$macos_min_ver
else
	meson_configure+=$'\t'-Dmanifest_date=$(date --iso-8601)
fi
meson_configure+=$'\t'-Dapp_name=$APP_NAME
meson_configure+=$'\t'-Dapp_comment=$APP_COMMENT
meson_configure+=$'\t'-Dapp_exe=$APP_EXE
meson_configure+=$'\t'-Dapp_id=$APP_ID
meson_configure+=$'\t'-Dapp_data=$APP_DATA
meson_configure+=$'\t'-Dapp_vendor=$APP_VENDOR
meson_configure+=$'\t'-Dstrip=false
meson_configure+=$'\t'-Db_staticpic=false
meson_configure+=$'\t'-Dmod_id=$MOD_ID
case $BSH_HOST_ARCH-$BSH_HOST_PLATFORM-$BSH_HOST_LIBC-$BSH_DEBUG_RELEASE in
x86_64-linux-gnu-debug) ;&
x86_64-windows-mingw-debug) ;&
x86_64-windows-msvc-debug) ;&
x86_64-darwin-macos-debug)
	meson_configure+=$'\t'-Dbuild_render=true
	meson_configure+=$'\t'-Dbuild_font=true
	;;
esac
if [[ $PACKAGE_MODE == nohttp ]]; then
	meson_configure+=$'\t'-Dhttp=false
fi
if [[ $PACKAGE_MODE == nolua ]]; then
	meson_configure+=$'\t'-Dlua=none
fi
if [[ $PACKAGE_MODE == backendvs ]]; then
	meson_configure+=$'\t'-Dbackend=vs
	# meson 1.2.3 configures vs projects that bring their own manifest, which conflicts with ours
	# TODO: remove this patch once https://github.com/mesonbuild/meson/pull/12472 makes it into a release that we can use
	meson_configure+=$'\t'-Dwindows_utf8cp=false
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
	if [[ "$BSH_HOST_PLATFORM-$BSH_HOST_LIBC $BSH_BUILD_PLATFORM" == "windows-mingw windows" ]]; then
		meson_configure+=$'\t'-Dworkaround_elusive_bzip2=true
		meson_configure+=$'\t'-Dworkaround_elusive_bzip2_include_dir=/ucrt64/include
		meson_configure+=$'\t'-Dworkaround_elusive_bzip2_lib_dir=/ucrt64/lib
	fi
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
if [[ $stable_or_beta == yes ]]; then
	xyz=$(echo $RELEASE_NAME | cut -d 'v' -f 2 | cut -d 'b' -f 1) # $RELEASE_NAME is vX.Y.Z or vX.Y.Zb
	display_version_major=$(echo $xyz | cut -d '.' -f 1)
	display_version_minor=$(echo $xyz | cut -d '.' -f 2)
	build_num=$(echo $xyz | cut -d '.' -f 3)
	if [[ $MOD_ID != 0 ]]; then
		meson_configure+=$'\t'-Ddisplay_version_major=$display_version_major
		meson_configure+=$'\t'-Ddisplay_version_minor=$display_version_minor
		meson_configure+=$'\t'-Dbuild_num=$build_num
	fi
fi
if [[ $RELEASE_TYPE == snapshot ]]; then
	build_num=$(echo $RELEASE_NAME | cut -d '-' -f 2) # $RELEASE_NAME is snapshot-X
	meson_configure+=$'\t'-Dsnapshot=true
	if [[ $MOD_ID != 0 ]]; then
		meson_configure+=$'\t'-Dbuild_num=$build_num
	fi
fi
if [[ $RELEASE_TYPE == snapshot ]] && [[ $MOD_ID != 0 ]]; then
	>&2 echo "mods and snapshots do not mix"
	exit 1
fi
if [[ $RELEASE_TYPE == snapshot ]] || [[ $MOD_ID != 0 ]]; then
	meson_configure+=$'\t'-Dupdate_server=starcatcher.us/TPT
	if [[ $BSH_HOST_PLATFORM == emscripten ]]; then
		meson_configure+=$'\t'-Dserver=tptserv.starcatcher.us
		meson_configure+=$'\t'-Dstatic_server=tptserv.starcatcher.us/Static
	fi
fi
if [[ $RELEASE_TYPE != dev ]]; then
	meson_configure+=$'\t'-Dignore_updates=false
fi
if [[ "$BSH_HOST_PLATFORM-$BSH_HOST_LIBC" == "windows-mingw" ]]; then
	meson_configure+=$'\t'--cross-file=.github/mingw-ghactions.ini
	# there is some mingw bug that only ever manifests on ghactions which makes MakeIco.exe use tons of memory and fail
	# TODO: remove this hack once we figure out how to fix that
	meson_configure+=$'\t'-Dwindows_icons=false
fi
if [[ $BSH_DEBUG_RELEASE-$BSH_STATIC_DYNAMIC == release-static ]]; then
	meson_configure+=$'\t'-Db_lto=true
fi
if [[ $BSH_HOST_PLATFORM-$BSH_HOST_ARCH == darwin-aarch64 ]]; then
	meson_configure+=$'\t'--cross-file=.github/macaa64-ghactions.ini
fi
if [[ $BSH_HOST_PLATFORM == emscripten ]]; then
	meson_configure+=$'\t'--cross-file=.github/emscripten-ghactions.ini
fi
if [[ $RELEASE_TYPE == tptlibsdev ]] && ([[ $BSH_HOST_PLATFORM-$BSH_HOST_LIBC == windows-msvc ]] || [[ $BSH_STATIC_DYNAMIC == static ]]); then
	if [[ -z ${TPTLIBSREMOTE-} ]]; then
		if [[ -z "${GITHUB_REPOSITORY_OWNER-}" ]]; then
			>&2 echo "GITHUB_REPOSITORY_OWNER not set"
			exit 1
		fi
		tptlibsremote=https://github.com/$GITHUB_REPOSITORY_OWNER/tpt-libs
	else
		tptlibsremote=$TPTLIBSREMOTE
	fi
	tptlibsbranch=$(echo $RELEASE_NAME | cut -d '-' -f 2-) # $RELEASE_NAME is tptlibsdev-BRANCH
	if [[ -d build-tpt-libs ]] && [[ ${TPTLIBSRESET-} == yes ]]; then
		rm -rf build-tpt-libs
	fi
	if [[ ! -d build-tpt-libs/tpt-libs ]]; then
		mkdir -p build-tpt-libs
		cd build-tpt-libs
		git clone $tptlibsremote --branch $tptlibsbranch --depth 1
		cd ..
	fi
	tpt_libs_vtag=v00000000000000
	if [[ ! -f build-tpt-libs/tpt-libs/.ok ]]; then
		cd build-tpt-libs/tpt-libs
		BSH_VTAG=$tpt_libs_vtag ./.github/build.sh
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
	android_platform_jar=$ANDROID_SDK_ROOT/platforms/$android_platform/android.jar
	if ! [[ -f $android_platform_jar ]]; then
		>&2 echo "$android_platform_jar not found"
		exit 1
	fi
	meson_configure+=$'\t'--cross-file=android/cross/$BSH_HOST_ARCH.ini
	cat << ANDROID_INI > .github/android-ghactions.ini
[constants]
andriod_ndk_toolchain_bin = '$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin'
andriod_sdk_build_tools = '$ANDROID_SDK_ROOT/build-tools/32.0.0'

[properties]
# android_ndk_toolchain_prefix comes from the correct cross-file in ./android/cross
android_ndk_toolchain_prefix = android_ndk_toolchain_prefix
android_platform = '$android_platform'
android_platform_jar = '$android_platform_jar'
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
fi
meson_configure+=$'\t'-Dc_args=[$c_args]
meson_configure+=$'\t'-Dcpp_args=[$c_args]
meson_configure+=$'\t'-Dc_link_args=[$c_link_args]
meson_configure+=$'\t'-Dcpp_link_args=[$c_link_args]
$meson_configure build
cd build

function verify_version_component() {
	local key=$1
	local expected=$2
	local actual=$(jq -r '.[] | select(.name == "'$key'") | .value' < meson-info/intro-buildoptions.json)
	if [[ $actual != $expected ]]; then
		>&2 echo "meson option $key expected to be $expected, is instead $actual"
		exit 1
	fi
}
if [[ $stable_or_beta == yes ]] && [[ $MOD_ID == 0 ]]; then
	verify_version_component display_version_major $display_version_major
	verify_version_component display_version_minor $display_version_minor
	verify_version_component build_num $build_num
	verify_version_component upstream_version_major $display_version_major
	verify_version_component upstream_version_minor $display_version_minor
	verify_version_component upstream_build_num $build_num
fi
if [[ $RELEASE_TYPE == snapshot ]] && [[ $MOD_ID == 0 ]]; then
	verify_version_component build_num $build_num
	verify_version_component upstream_build_num $build_num
fi

strip=strip
objcopy=objcopy
strip_target=$ASSET_PATH
if [[ $BSH_HOST_PLATFORM == android ]]; then
	strip=$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-$strip
	objcopy=$ANDROID_NDK_LATEST_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-$objcopy
	strip_target=lib$APP_EXE.so
fi
if [[ $PACKAGE_MODE == appimage ]]; then
	# so far this can only happen with $BSH_HOST_PLATFORM-$BSH_HOST_LIBC == linux-gnu, but this may change later
	meson configure -Dcan_install=no -Dignore_updates=true -Dbuild_render=false -Dbuild_font=false
	strip_target=$APP_EXE
fi
meson_compile=meson$'\t'compile
meson_compile+=$'\t'-v
if [[ $BSH_BUILD_PLATFORM == windows ]] && [[ $PACKAGE_MODE != backendvs ]]; then
	set +e
	meson_compile+=$'\t'--ninja-args='["-d","keeprsp"]'
	$meson_compile
	ninja_code=$?
	set -e
	cat $APP_EXE.exe.rsp
	[[ $ninja_code == 0 ]];
	echo # rsps don't usually have a newline at the end
	if [[ "$BSH_HOST_PLATFORM-$BSH_STATIC_DYNAMIC $BSH_BUILD_PLATFORM" == "windows-dynamic windows" ]]; then
		# on windows we provide the dynamic dependencies also; makes sense to check for their presence
		# msys ldd works fine but only on windows build machines
		if ldd $APP_EXE | grep "not found"; then
			exit 1 # ldd | grep will have printed missing deps
		fi
	fi
else
	$meson_compile
fi

if [[ $SEPARATE_DEBUG == yes ]] && [[ $BSH_HOST_PLATFORM-$BSH_HOST_LIBC != windows-msvc ]]; then
	$objcopy --only-keep-debug $strip_target $DEBUG_ASSET_PATH
	$strip --strip-debug --strip-unneeded $strip_target
	$objcopy --add-gnu-debuglink $DEBUG_ASSET_PATH $strip_target
	chmod -x $DEBUG_ASSET_PATH
fi
if [[ $BSH_HOST_PLATFORM == android ]]; then
	$JAVA_HOME_8_X64/bin/keytool -genkeypair -keystore keystore.jks -alias androidkey -validity 10000 -keyalg RSA -keysize 2048 -keypass bagelsbagels -storepass bagelsbagels -dname "CN=nobody"
	meson configure -Dandroid_keystore=$(realpath keystore.jks)
	ANDROID_KEYSTORE_PASS=bagelsbagels ninja android/$APP_EXE.apk
	mv android/$APP_EXE.apk $APP_EXE.apk
fi
if [[ $PACKAGE_MODE == dmg ]]; then
	# so far this can only happen with $BSH_HOST_PLATFORM-$BSH_HOST_LIBC == darwin-macos
	appdir=$APP_NAME.app
	mkdir $appdir
	mkdir $appdir/Contents
	cp resources/Info.plist $appdir/Contents/Info.plist
	mkdir $appdir/Contents/MacOS
	cp $APP_EXE $appdir/Contents/MacOS/$APP_EXE
	mkdir $appdir/Contents/Resources
	mkdir icon_exe.iconset
	cp ../resources/generated_icons/icon_exe_16.png icon_exe.iconset/icon_16x16.png
	cp ../resources/generated_icons/icon_exe_32.png icon_exe.iconset/icon_32x32.png
	cp ../resources/generated_icons/icon_exe.png    icon_exe.iconset/icon_128x128.png
	iconutil -c icns icon_exe.iconset
	cp icon_exe.icns $appdir/Contents/Resources/icon_exe.icns
	mkdir icon_cps.iconset
	cp ../resources/generated_icons/icon_cps_16.png icon_cps.iconset/icon_16x16.png
	cp ../resources/generated_icons/icon_cps_32.png icon_cps.iconset/icon_32x32.png
	cp ../resources/generated_icons/icon_cps.png    icon_cps.iconset/icon_128x128.png
	iconutil -c icns icon_cps.iconset
	cp icon_cps.icns $appdir/Contents/Resources/icon_cps.icns
	mkdir dmgroot
	mv $appdir dmgroot/$appdir
	cp ../LICENSE dmgroot/LICENSE
	cp ../README.md dmgroot/README.md
	hdiutil create -format UDZO -volname $APP_NAME -fs HFS+ -srcfolder dmgroot -o $ASSET_PATH
elif [[ $PACKAGE_MODE == emscripten ]]; then
	tar cvf $ASSET_PATH $APP_EXE.js $APP_EXE.worker.js $APP_EXE.wasm
elif [[ $PACKAGE_MODE == appimage ]]; then
	# so far this can only happen with $BSH_HOST_PLATFORM-$BSH_HOST_LIBC == linux-gnu, but this may change later
	case $BSH_HOST_ARCH in
	aarch64) appimage_arch=aarch64;;
	arm)     appimage_arch=armhf  ;;
	x86)     appimage_arch=i686   ;;
	x86_64)  appimage_arch=x86_64 ;;
	esac
	curl -fsSLo appimagetool "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-$appimage_arch.AppImage"
	curl -fsSLo AppRun "https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-$appimage_arch"
	chmod +x appimagetool
	chmod +x AppRun
	appdir=bagels.AppDir # doesn't matter, won't be visible in the resulting appimage
	mkdir -p $appdir/usr/bin
	mkdir -p $appdir/usr/share/metainfo
	mkdir -p $appdir/usr/share/applications
	mkdir -p $appdir/usr/share/icons
	cp $APP_EXE $appdir/usr/bin/$APP_EXE
	mv AppRun $appdir/AppRun
	cp ../resources/icon_exe.svg $appdir/$APP_VENDOR-$APP_EXE.svg
	cp resources/powder.desktop $appdir/$APP_ID.desktop
	cp resources/appdata.xml $appdir/usr/share/metainfo/$APP_ID.appdata.xml
	cp $appdir/$APP_VENDOR-$APP_EXE.svg $appdir/usr/share/icons/$APP_VENDOR-$APP_EXE.svg
	cp $appdir/$APP_ID.desktop $appdir/usr/share/applications/$APP_ID.desktop
	./appimagetool $appdir $ASSET_PATH
fi
