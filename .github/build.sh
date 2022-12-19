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
*) >&2 echo "configuration $BSH_HOST_ARCH-$BSH_HOST_PLATFORM-$BSH_HOST_LIBC-$BSH_STATIC_DYNAMIC is not supported" && exit 1;;
esac

function inplace_sed() {
	local subst=$1
	local path=$2
	if [[ $BSH_BUILD_PLATFORM == darwin ]]; then
		sed -i "" -e $subst $path
	else
		sed -i $subst $path
	fi
}

function subst_version() {
	local path=$1
	if [[ $BSH_HOST_PLATFORM != darwin ]]; then
		inplace_sed "s|SUBST_DATE|$(date --iso-8601)|g" $path
	fi
	inplace_sed "s|SUBST_SAVE_VERSION|$save_version|g" $path
	inplace_sed "s|SUBST_MINOR_VERSION|$minor_version|g" $path
	inplace_sed "s|SUBST_BUILD_NUM|$build_num|g" $path
}

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
	VS_ENV_PARAMS=$vs_env_arch$'\t'-vcvars_ver=14.1
	. ./.github/vs-env.sh
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

meson_configure=meson$'\t'setup
if [[ $BSH_DEBUG_RELEASE == release ]]; then
	meson_configure+=$'\t'-Dbuildtype=debugoptimized
fi
meson_configure+=$'\t'-Db_strip=false
meson_configure+=$'\t'-Db_staticpic=false
meson_configure+=$'\t'-Dinstall_check=true
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
save_version=$(grep -w src/Config.template.h -e "#define SAVE_VERSION" | cut -d ' ' -f 3)
minor_version=$(grep -w src/Config.template.h -e "#define MINOR_VERSION" | cut -d ' ' -f 3)
build_num=$(grep -w src/Config.template.h -e "#define BUILD_NUM" | cut -d ' ' -f 3)
if [[ $stable_or_beta == yes ]] && [[ $MOD_ID != 0 ]]; then
	save_version=$(echo $RELEASE_NAME | cut -d '.' -f 1)
	minor_version=$(echo $RELEASE_NAME | cut -d '.' -f 2)
	build_num=$(echo $RELEASE_NAME | cut -d '.' -f 3)
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
fi
if [[ $BSH_HOST_PLATFORM-$BSH_HOST_LIBC != windows-mingw ]] && [[ $BSH_STATIC_DYNAMIC == static ]]; then
	# LTO simply doesn't work with MinGW. I have no idea why and I also don't care.
	# It also has a tendency to not play well with dynamic libraries.
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
	meson configure -Dinstall_check=false -Dignore_updates=true -Dbuild_render=false -Dbuild_font=false
	strip_target=$APP_EXE
fi
if [[ $BSH_BUILD_PLATFORM == windows ]]; then
	set +e
	ninja -v -d keeprsp
	ninja_code=$?
	set -e
	cat $APP_EXE.exe.rsp
	[[ $ninja_code == 0 ]];
else
	ninja -v
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
	subst_version $appdir/Contents/Info.plist
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
	hdiutil create uncompressed.dmg -ov -volname $APP_NAME -fs HFS+ -srcfolder dmgroot
	hdiutil convert uncompressed.dmg -format UDZO -o $ASSET_PATH
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
	subst_version $appdir/usr/share/metainfo/$APP_ID.appdata.xml
	cp $appdir/$APP_VENDOR-$APP_EXE.svg $appdir/usr/share/icons/$APP_VENDOR-$APP_EXE.svg
	cp $appdir/$APP_ID.desktop $appdir/usr/share/applications/$APP_ID.desktop
	./appimagetool $appdir $ASSET_PATH
fi
