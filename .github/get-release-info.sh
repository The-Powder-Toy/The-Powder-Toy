value=`cat release_url/release_url.txt`
echo ::set-output name=upload_url::$value
