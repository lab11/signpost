Fake Radio Receiver
===========

This package interacts with the debug_backplane radio module. It receives
http post commands from the radio, deserializes the commands,
and either performs an http post with the parameters, or, if the 
URL inidates the GDP url, performs a GDP append.

Installation
===========
sudo pip install signpost-debug-radio

GDP Installation
===============
To use the GDP append feature, you must install GDP on your computer.
Currently this is only supported for Debian based machines.

From this directory:

```bash
$ cd gdp-packages
$ sudo dpkg - i gdp-client_0.7.2-1_amd64.deb python-gdp_0.7.2_all.deb
$ sudo apt-get -f install
$ sudo dpkg - i gdp-client_0.7.2-1_amd64.deb python-gdp_0.7.2_all.deb
```

After theses packages are installed, running signpost-debug-radio
should stop giving you warnings about missing the GDP packages.

Usage
=====
signpost-debug-radio is compatible with the signpost_networking_post
API implementation. It receives the serialized version of the signpost_networking_post
command and performs a post with those parameters.

##Running signpost-debug-radio
Signpost-debug-radio will try to automatically detect if you are plugged
into the debug-radio USB port on the debugging backplane. You can also
specify the port name manually by running

```
signpost-debug-radio --port <port_name>
```

##Appending to GDP
To perform GDP appends the signpost_networking_post URL must be set to

```
gdp.lab11.eecs.umich.edu/gdp/v1/<log_name>/append
```

If the log does not already exist, signpost-debug-radio will create
the log for you before appending data. The signpost-debug-radio script
will print 1) whether the log existed and 2) if the append was successful.


Upload to PyPI
--------------
Internal note.

    python3 setup.py sdist
    twine upload dist/signpost-debug-radio-X.X.X.tar.gz
