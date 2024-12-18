#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

host=$(echo "$PUBLISH_HOSTPORT" | cut -d ':' -f 1)
port=$(echo "$PUBLISH_HOSTPORT" | cut -d ':' -f 2)

set +e
mkdir ~/.ssh
chmod 700 ~/.ssh
touch ~/.ssh/known_hosts
chmod 600 ~/.ssh/known_hosts
set -e

echo "[$host]:$port ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIDsmJkDd7Rxnuuf4kpbJCDZvkO03lp4lmpzGmFW6LCqG" >> ~/.ssh/known_hosts

commands="quit"$'\n'
if [[ -z ${PUBLISH_ACCESSCHECK-} ]]; then
	commands="put $PUBLISH_FILENAME"$'\n'"$commands"
	commands="cd ${PUBLISH_DIRECTORY:-.}"$'\n'"$commands"
fi
SSHPASS="$PUBLISH_PASSWORD" sshpass -e sftp -oPort="$port" -oBatchMode=no -b - "$PUBLISH_USERNAME@$host" <<< "$commands"
