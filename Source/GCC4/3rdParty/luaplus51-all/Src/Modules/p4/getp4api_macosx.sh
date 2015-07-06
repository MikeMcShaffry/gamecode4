rm -rf p4api
mkdir p4api
if [ -f p4api.tgz ] then
	rm p4api.tgz
fi
curl ftp://ftp.perforce.com/perforce/r10.2/bin.macosx105u/p4api.tgz -o p4api.tgz
tar xvfz p4api.tgz
mv -f p4api-2010.2.277537 p4api

