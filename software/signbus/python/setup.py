#!/usr/bin/env python3

# Save people like Pat from themselves:
import sys
if sys.version_info < (3,0):
    sys.exit('Sorry, Python < 3.0 is not supported')

# Grab version
from signpost import __version__ as version

from setuptools import setup

# On the package maintainer side, need pypandoc because I don't grok rst at all,
# but no need to make an actual package dependency
try:
    import pypandoc
    long_description = pypandoc.convert_file('README.md', 'rst'),
except ImportError:
    if ('upload' in sys.argv) or ('sdist' in sys.argv) or ('bdist' in sys.argv):
        raise
    long_description = None

setup(
    name = 'signpost',
    packages = ['signpost'],
    version = version,
    description = 'Interface library for signpost networks',
    author = 'Pat Pannuto',
    author_email = 'pat.pannuto@gmail.com',
    url = 'https://github.com/lab11/signpost',
    # download_url = 'https://.../python3-signpost-VERSION.tgz'
    keywords = ['signpost', 'embedded'],
    classifiers = [
        "Development Status :: 3 - Alpha",
        "Environment :: Other Environment",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        #"Operating System :: OS Independent", unsure - check periphery?
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.3",
        "Programming Language :: Python :: 3.4",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Topic :: Software Development :: Embedded Systems",
        ],
    long_description = long_description,
    install_requires = [
        'cryptography',
        'periphery',
        ],
    )

