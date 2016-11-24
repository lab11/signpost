#!/usr/bin/env python3

import argparse
import ssl
import urllib
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

context = ssl._create_unverified_context()
cipher_list_rsa = open('cipher_list_rsa.txt').read().strip().split('\n')
cipher_list_ecdsa = open('cipher_list_ecdsa.txt').read().strip().split('\n')

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

def run_benchmarks (key, length):
    if key == 'rsa':
        cipher_list = cipher_list_rsa
    elif key == 'ecdsa':
        cipher_list = cipher_list_ecdsa
    for cipher in cipher_list:
        print ('Now running test with ' + length + ' ' + key + ' cipher suite ' + cipher)
        #restart server with new cipher suite
        s.sendline('openssl s_server -key /home/debian/cryptobench/key_' + key + '_' + length + '.pem -cert /home/debian/cryptobench/cert_' + key + '_' + length + '.pem -www -cipher ' + cipher + ' &')
        s.sendline()
        s.prompt ()
        s.sendline('pid=$!')
        #trigger oscope
        input ('Ready?')
        #https get request
        response = urllib.request.urlopen('https://' + args.host + ':4433', context=context)
        print(response.read())
        input ('Done?')
        s.sendline('kill ${pid}')


if not s.login (args.host, args.username, args.password):
    print ('SSH session failed to login')
    print (str(s))
    exit(1)

print ('SSH session login successful')
get_sudo(s, args.password)
print ('Begin ECDSA TLS handshake benchmarks')
run_benchmarks('ecdsa', '256')
run_benchmarks('ecdsa', '384')
run_benchmarks('ecdsa', '521')
print ('Begin RSA TLS handshake benchmarks')
run_benchmarks('rsa', '2048')
run_benchmarks('rsa', '3072')
run_benchmarks('rsa', '4096')

#exit sudo
s.sendline ('exit')
s.prompt ()
s.logout ()
