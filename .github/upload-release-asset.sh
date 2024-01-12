set -euo pipefail
IFS=$'\t\n'

temp=.temp
mkdir $temp
cp $ASSET_PATH $temp/$ASSET_NAME
(
	cd $temp
	gh release upload $GITHUB_REF_NAME $ASSET_NAME
)
rm -r $temp
echo browser_download_url=https://github.com/$GITHUB_REPOSITORY/releases/download/$GITHUB_REF_NAME/$ASSET_NAME >> $GITHUB_OUTPUT
