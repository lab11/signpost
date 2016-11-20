#!/usr/bin/env python3

import saleae
import argparse
import sys
import requests
from pexpect import pxssh
import re
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('host', help='host ip address')
parser.add_argument('username', help='ssh username')
parser.add_argument('password', help='ssh password')
args = parser.parse_args()

s = pxssh.pxssh()

cipher_list = open('cipher_list.txt').read().strip().split(':')
print(cipher_list)

def get_sudo (session, password):
    session.sendline ('sudo -s')
    rootprompt = re.compile(b'.*[$#]')
    i = session.expect([rootprompt, 'assword.*: '])
    if i==0:
        print ('No password was needed')
        pass
    elif i==1:
        print ('Sending password')
        session.sendline(password)
        j = session.expect([rootprompt, 'Sorry, try again'])
        if j:
            raise Exception('get_sudo: Bad Password')
        else: pass
    else:
        raise Exception('unexpected output')
    session.set_unique_prompt()

if not s.login (args.host, args.username, args.password):
    print ('SSH session failed to login')
    print (str(s))
else:
    print ('SSH session login successful')
    get_sudo(s, args.password)
    for cipher in cipher_list:
        print ('Now running test with cipher suite ' + cipher)
        #change config to support cipher
        #restart apache
        s.sendline ('systemctl restart apache2')
        s.prompt ()
        #trigger saleae
        #https get request
    s.sendline ('exit')
    s.prompt ()
    s.logout ()
