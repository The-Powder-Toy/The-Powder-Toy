#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

LFTP_PASSWORD=$PUBLISH_PASSWORD lftp -c "open --user '$PUBLISH_USERNAME' --env-password -e 'set ftp:ssl-protect-data true; set ssl:verify-certificate false; cd "${PUBLISH_DIRECTORY:-.}"; put \"$PUBLISH_FILENAME\";' ftp://$PUBLISH_HOSTPORT"
