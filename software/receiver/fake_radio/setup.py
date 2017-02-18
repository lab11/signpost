from setuptools import setup

# Save people like Pat from themselves:
import sys
if sys.version_info > (3,0):
    sys.exit('Sorry, Python3 is not supported')

import re
VERSIONFILE="signpost_fake_radio/_version.py"
verstrline = open(VERSIONFILE, "rt").read()
VSRE = r"^__version__ = ['\"]([^'\"]*)['\"]"
mo = re.search(VSRE, verstrline, re.M)
if mo:
    verstr = mo.group(1)
else:
    raise RuntimeError("Unable to find version string in %s." % (VERSIONFILE,))

setup(name='signpost_fake_radio',
      version=verstr,
      description='Signpost Fake Radio Interface',
      author='Brad Campbell',
      author_email='bradjc@umich.edu',
      url='https://github.com/lab11/signpost',
      packages=['signpost_fake_radio'],
      entry_points={
        'console_scripts': [
          'signpost_fake_radio = signpost_fake_radio.main:main'
        ]
      },
      install_requires=["pyserial >= 3.0.1"],
     )
