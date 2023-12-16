set -euo pipefail
IFS=$'\t\n'

echo 'C:\msys64\ucrt64\bin' >> tmp
echo 'C:\msys64\usr\bin' >> tmp
cat $GITHUB_PATH >> tmp
mv tmp $GITHUB_PATH

echo "MSYSTEM=UCRT64" >> $GITHUB_ENV
echo "PKG_CONFIG="'C:\msys64\ucrt64\bin\pkg-config.exe' >> $GITHUB_ENV
