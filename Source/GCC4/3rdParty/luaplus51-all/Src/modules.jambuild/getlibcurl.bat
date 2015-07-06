pushd
cd ..\Modules\lua-curl\src
if exist curl-7.21.6.zip del curl-7.21.6.zip
if exist curl-7.21.6 rmdir /s /q curl-7.21.6
wget http://curl.haxx.se/download/curl-7.21.6.zip
unzip curl-7.21.6.zip
del curl-7.21.6.zip
popd
