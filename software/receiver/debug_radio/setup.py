from setuptools import setup

import re
VERSIONFILE="signpost_debug_radio/_version.py"
verstrline = open(VERSIONFILE, "rt").read()
VSRE = r"^__version__ = ['\"]([^'\"]*)['\"]"
mo = re.search(VSRE, verstrline, re.M)
if mo:
    verstr = mo.group(1)
else:
    raise RuntimeError("Unable to find version string in %s." % (VERSIONFILE,))

setup(name='signpost_debug_radio',
      version=verstr,
      description='Signpost Debug Radio Interface',
      author='Brad Campbell',
      author_email='bradjc@umich.edu',
      url='https://github.com/lab11/signpost',
      packages=['signpost_debug_radio'],
      entry_points={
        'console_scripts': [
          'signpost-debug-radio = signpost_debug_radio.main:main'
        ]
      },
      install_requires=[
          "future >= 0.15.2",
          "pyserial >= 3.0.1",
          ],
     )
