if [ ! -z "$STARCATCHER_PUBLISH_HOSTPORT" ]; then
	echo ::set-output name=can_publish::yes
else
	echo ::set-output name=can_publish::no
fi
