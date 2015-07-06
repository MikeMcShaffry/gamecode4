if exist p4api\vs2008\bin.ntx86 rmdir /s /q p4api\vs2008\bin.ntx86
mkdir p4api\vs2008\bin.ntx86
cd p4api\vs2008\bin.ntx86
if exist p4api_vs2008_dyn.zip del p4api_vs2008_dyn.zip
wget ftp://ftp.perforce.com/perforce/r10.2/bin.ntx86/p4api_vs2008_dyn.zip
unzip p4api_vs2008_dyn.zip
ren p4api-2010.2.295040-vs2008_dyn release
del p4api_vs2008_dyn.zip

if exist p4api_vs2008_dyn_vsdebug.zip del p4api_vs2008_dyn_vsdebug.zip
wget ftp://ftp.perforce.com/perforce/r10.2/bin.ntx86/p4api_vs2008_dyn_vsdebug.zip
unzip p4api_vs2008_dyn_vsdebug.zip
ren p4api-2010.2.295040-vs2008_dyn_vsdebug debug
del p4api_vs2008_dyn_vsdebug.zip

cd ..\..\..

if exist p4api\vs2008\bin.ntx64 rmdir /s /q p4api\vs2008\bin.ntx64
mkdir p4api\vs2008\bin.ntx64
cd p4api\vs2008\bin.ntx64
if exist p4api_vs2008_dyn.zip del p4api_vs2008_dyn.zip
wget ftp://ftp.perforce.com/perforce/r10.2/bin.ntx64/p4api_vs2008_dyn.zip
unzip p4api_vs2008_dyn.zip
ren p4api-2010.2.295040-vs2008_dyn release
del p4api_vs2008_dyn.zip

if exist p4api_vs2008_dyn_vsdebug.zip del p4api_vs2008_dyn_vsdebug.zip
wget ftp://ftp.perforce.com/perforce/r10.2/bin.ntx64/p4api_vs2008_dyn_vsdebug.zip
unzip p4api_vs2008_dyn_vsdebug.zip
ren p4api-2010.2.295040-vs2008_dyn_vsdebug debug
del p4api_vs2008_dyn_vsdebug.zip

cd ..\..\..

