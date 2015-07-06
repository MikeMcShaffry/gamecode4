pushd
cd ..\Modules\lrexlib
if exist pcre-7.8.zip del pcre-7.8.zip
if exist pcre-7.8 rmdir /s /q pcre-7.8
if exist pcre-8.00.zip del pcre-8.00.zip
if exist pcre-8.00 rmdir /s /q pcre-8.00
if exist pcre-8.02.zip del pcre-8.02.zip
if exist pcre-8.02 rmdir /s /q pcre-8.02
wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.02.zip
unzip pcre-8.02.zip
del pcre-8.02.zip

cd pcre-8.02
if not exist pcre.h copy pcre.h.generic pcre.h
if not exist config.h copy config.h.generic config.h
if not exist pcre_chartables.c copy pcre_chartables.c.dist pcre_chartables.c
cd ..
popd
