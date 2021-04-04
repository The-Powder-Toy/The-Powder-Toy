#!/bin/sh

cd "$STARCATCHER_PUBLISH_FILENAME"
mv "$1" "$STARCATCHER_PUBLISH_FILENAME"
LFTP_PASSWORD=$STARCATCHER_PUBLISH_PASSWORD lftp -c "open --user '$STARCATCHER_PUBLISH_USERNAME' --env-password -e 'set ftp:ssl-protect-data true; set ssl:verify-certificate false; put \"$STARCATCHER_PUBLISH_FILENAME\";' ftp://$STARCATCHER_PUBLISH_HOSTPORT"
