#!/usr/bin/env bash

rm -fr _build

cordova create _build edu.umich.eecs.lab11.solartest.demo SolarTest
pushd _build
cordova platform add android
cordova plugin add cordova-plugin-whitelist
cordova plugin add cordova-plugin-console
cordova plugin add cordova-plugin-ble-central

pushd www

rm -r css
rm -r img
rm -r js
rm index.html

ln -s ../../css .
ln -s ../../js .
ln -s ../../index.html .
ln -s ../../index.html .
ln -s ../../cordova_android.js .
ln -s ../../cordova_ios.js .
ln -s ../../cordova_plugins.js .
ln -s ../../cordova.js cordova_test.js

popd

cordova build

popd
