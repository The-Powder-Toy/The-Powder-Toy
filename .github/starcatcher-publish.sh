#!/usr/bin/env bash

cp "$ASSET_PATH" "$PUBLISH_FILENAME"
LFTP_PASSWORD=$PUBLISH_PASSWORD lftp -c "open --user '$PUBLISH_USERNAME' --env-password -e 'set ftp:ssl-protect-data true; set ssl:verify-certificate false; put \"$PUBLISH_FILENAME\";' ftp://$PUBLISH_HOSTPORT"
