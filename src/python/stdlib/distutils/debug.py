import os

__revision__ = "$Id: debug.py 68943 2009-01-25 22:09:10Z tarek.ziade $"

# If DISTUTILS_DEBUG is anything other than the empty string, we run in
# debug mode.
DEBUG = os.environ.get('DISTUTILS_DEBUG')
