#!/bin/sh

curl $1 > perform_release
if ! grep "Release complete" < perform_release > /dev/null; then
	cat perform_release
	exit 1
fi
