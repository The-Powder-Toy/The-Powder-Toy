set -euo pipefail
IFS=$'\t\n'

IFS=$'\t\n\r'
for i in $("$(env | grep '^ProgramFiles(x86)=' | cut -d = -f 2-)/Microsoft Visual Studio/Installer/vswhere.exe" \
	-sort \
	-prerelease \
	-requiresAny \
	-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
	-requires Microsoft.VisualStudio.Workload.WDExpress \
	-products \* \
	-utf8 \
	-property installationPath); do
	if ! [ -z ${VS_ENV_FILTER-} ]; then
		if ! echo $i | grep $VS_ENV_FILTER >/dev/null; then
			continue
		fi
	fi
	vs_install_dir=$i
	break
done
IFS=$'\t\n'

for i in $(MSYS_NO_PATHCONV=1 cmd /c "$vs_install_dir\\VC\\Auxiliary\\Build\\vcvarsall.bat" $VS_ENV_PARAMS \& env \& exit /b); do
	set +e
	export "$i" 2>/dev/null
	echo $i | grep ERROR
	set -e
done

cl
