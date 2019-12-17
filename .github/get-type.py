import base64
import re
import sys

ref = sys.argv[1]

match_stable = re.match(r'refs/tags/v([0-9]+)\.([0-9]+)\.([0-9]+)', ref)
match_snapshot = re.match(r'refs/tags/snapshot-([0-9]+)', ref)
if match_stable:
	print('::set-output name=TYPE::stable')
	print('::set-output name=NAME::v%s.%s.%s' % (match_stable.group(1), match_stable.group(2), match_stable.group(3)))
	print('::set-output name=RELTYPECFG::%s' % base64.b64encode(('-Dignore_updates=false\t-Dversion_major=%s\t-Dversion_minor=%s\t-Dversion_build=%s' % (match_stable.group(1), match_stable.group(2), match_stable.group(3))).encode('utf-8')).decode('utf-8'))
elif match_snapshot:
	print('::set-output name=TYPE::snapshot')
	print('::set-output name=NAME::snapshot-%s' % match_snapshot.group(1))
	print('::set-output name=RELTYPECFG::%s' % base64.b64encode(('-Dignore_updates=false\t-Dsnapshot=true\t-Dsnapshot_id=%s' % match_snapshot.group(1)).encode('utf-8')).decode('utf-8'))
else:
	print('::set-output name=TYPE::dev')
	print('::set-output name=NAME::dev')
	print('::set-output name=RELTYPECFG::%s' % base64.b64encode(('-Dignore_updates=true').encode('utf-8')).decode('utf-8'))
