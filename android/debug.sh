#!/usr/bin/env bash

# mainly based on https://www.sh-zam.com/2019/05/debugging-krita-on-android.html

set -euo pipefail
IFS=$'\n\t'

function get_buildoption() {
	jq -r '.[] | select(.name == "'$1'") | .value' < meson-info/intro-buildoptions.json
}

# customize
default_app_id=uk.co.powdertoy.tpt
default_app_exe=powder
in_build_site=no
if which jq >/dev/null && [[ -f meson-info/intro-buildoptions.json ]]; then
	# pwd is most likely a build site
	in_build_site=yes
	default_app_id=$(get_buildoption app_id)
	default_app_exe=$(get_buildoption app_exe)
fi
app_id=${APP_ID:-$default_app_id}
app_exe=${APP_EXE:-$default_app_exe}
lldb_server=${LLDB_SERVER:-/opt/android-ndk/toolchains/llvm/prebuilt/linux-x86_64/lib/clang/17/lib/linux/aarch64/lldb-server}
lldb_server_port=${LLDB_SERVER_PORT:-9998}
jdb_port=${JDB_PORT:-13456}
lldb_client=${LLDB_CLIENT:-/opt/android-ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/lldb.sh}
adb=${ADB:-adb}
jdb=${JDB:-jdb}

# don't customize unless necessary
app_activity=${APP_ACTIVITY:-PowderActivity}
lldb_server_staging=${LLDB_SERVER_STAGING:-/data/local/tmp/lldb-server}
lldb_server_remote=${LLDB_SERVER_REMOTE:-lldb-server}
pidof_retry_count=${PIDOF_RETRY_COUNT:-20}
pidof_retry_delay=${PIDOF_RETRY_DELAY:-0.1}

function check_which() {
	if ! which $1 >/dev/null; then
		>&2 echo "[-] can't run $1"
		return 1
	fi
}

function check_env() {
	if ! [[ -f $lldb_server ]]; then
		>&2 echo "[-] $lldb_server doesn't exist"
		return 1
	fi
	check_which $lldb_client
	check_which $adb
	check_which $jdb
}

function check_adb() {
	$adb shell whoami >/dev/null
}

function maybe_install_app() {
	if [[ $in_build_site != yes ]]; then
		>&2 echo "[+] not in a build site, not adb installing anything"
		return 0
	fi
	android_keystore=$(get_buildoption android_keystore)
	android_keyalias=$(get_buildoption android_keyalias)
	if [[ -z ${ANDROID_KEYSTORE_PASS-} ]]; then
		>&2 cat << HELP
The current directory seems to be a build site, but ANDROID_KEYSTORE_PASS is not set, so android/$app_exe.apk cannot be invoked. If you don't have a keystore yet, create one with:

  ANDROID_KEYSTORE_PASS=bagelsbagels keytool -genkey \\
    -keystore $android_keystore \\
    -alias $android_keyalias \\
    -storepass:env ANDROID_KEYSTORE_PASS \\
    -keypass:env ANDROID_KEYSTORE_PASS \\
    -dname CN=bagels

Then try again with:

  ANDROID_KEYSTORE_PASS=bagelsbagels $0

Naturally, replace bagelsbagels with an appropriate password.
HELP
		exit 1
	fi
	>&2 echo "[+] adb installing android/$app_exe.apk"
	if ! $adb install android/$app_exe.apk; then
		>&2 echo "[-]   failed"
		return 1
	fi
}

function check_debuggable() {
	$adb shell run-as $app_id whoami >/dev/null
}

function find_lldb_server() {
	$adb shell run-as $app_id pgrep $lldb_server_remote >/dev/null
}

function kill_lldb_server() {
	if ! $adb shell run-as $app_id pkill $lldb_server_remote; then
		>&2 echo "[-]   failed"
		return 1
	fi
}

function adb_forward() {
	>&2 echo "[+] adb forwarding tcp:$jdb_port jdwp:$pid"
	if ! ($adb forward tcp:$jdb_port jdwp:$pid | grep $jdb_port >/dev/null); then
		>&2 echo "[+]   failed"
		return 1
	fi
}

function adb_unforward() {
	$adb forward --remove tcp:$jdb_port
}

function undo_current_adb_forward() {
	>&2 echo "[+] adb un-forwarding orphaned tcp:$jdb_port"
	adb_unforward
}

function maybe_undo_previous_adb_forward() {
	if $adb forward --list | grep tcp:$jdb_port; then
		>&2 echo "[+] adb un-forwarding orphaned tcp:$jdb_port"
		adb_unforward
	fi
}

function maybe_kill_previous_lldb_server() {
	if find_lldb_server; then
		>&2 echo "[+] killing orphaned $lldb_server_remote"
		kill_lldb_server
	fi
}

function kill_current_lldb_server() {
	>&2 echo "[+] killing $lldb_server_remote"
	kill_lldb_server
}

function start_app() {
	>&2 echo "[+] starting $app_id/.$app_activity"
	set +e
	$adb shell am start -D -n "$app_id/.$app_activity" >/dev/null
	set -e
	local i
	local maybe_pid
	local pidof_result
	for ((i = 0; i <= $pidof_retry_count; i++)); do
		set +e
		maybe_pid=$($adb shell pidof $app_id)
		pidof_result=$?
		set -e
		if [[ $pidof_result == 0 ]]; then
			pid=$maybe_pid
			break
		fi
		sleep $pidof_retry_delay
	done
	if [[ -z ${pid-} ]]; then
		>&2 echo "[-]   failed"
		return 1
	fi
	echo $pid
}

function jdb_attach() {
	>&2 echo "[+] attaching jdb in the background"
	$jdb -attach localhost:$jdb_port >/dev/null 2>/dev/null &
	disown $!
	# at some point jdb exits because it doesn't have an stdin... fine by me
}

function maybe_deploy_lldb_server() {
	if ! $adb shell [[ -f $lldb_server_staging ]]; then
		>&2 echo "[+] $lldb_server_remote not present on host, deploying"
		if ! ($adb push $lldb_server $lldb_server_staging && $adb shell chmod +x $lldb_server_staging); then
			>&2 echo "[-]   failed"
		fi
	fi
}

function start_lldb_server() {
	if ! $adb shell run-as $app_id pgrep $lldb_server_remote >/dev/null; then
		>&2 echo "[+] $lldb_server_remote not running on host, starting"
		$adb shell run-as $app_id cp $lldb_server_staging /data/data/$app_id/$lldb_server_remote
		$adb shell run-as $app_id ./$lldb_server_remote platform --server --listen "*:$lldb_server_port" >/dev/null 2>/dev/null &
		disown $!
		if ! $adb shell run-as $app_id pgrep $lldb_server_remote >/dev/null; then
			>&2 echo "[-]   failed"
			return 1
		fi
	fi
}

function start_lldb() {
	local pid=$1
	>&2 echo "[+] starting $lldb_client"
	local lldb_init=$(mktemp)
	cat - << LLDB_INIT > $lldb_init
platform select remote-android
platform connect connect://localhost:$lldb_server_port
attach $pid
continue
LLDB_INIT
	local lldb_status
	set +e
	$lldb_client --source $lldb_init
	lldb_status=$?
	set -e
	>&2 echo "[+] $lldb_client exited with status $lldb_status"
	rm $lldb_init
}

check_env
check_adb
maybe_install_app
check_debuggable
maybe_kill_previous_lldb_server
maybe_undo_previous_adb_forward
if [[ ${1-} == clean ]]; then
	>&2 echo "[+] done"
	exit 0
fi
maybe_deploy_lldb_server
start_lldb_server
pid=$(start_app)
adb_forward
jdb_attach
start_lldb $pid
kill_current_lldb_server
undo_current_adb_forward
>&2 echo "[+] done"
