cd ../Modules/lrexlib
if [ -f pcre-8.02.tar.bz2 ]
then
	rm pcre-8.02.tar.bz2
fi
if [ -f pcre-8.00.tar.bz2 ]
then
	rm pcre-8.00.tar.bz2
fi
if [ -f pcre-7.8.tar.bz2 ]
then
	rm pcre-7.8.tar.bz2
fi
if [ -d pcre-7.8 ]
then
	rm -rf pcre-7.8
fi
curl ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.02.tar.bz2 -o pcre-8.02.tar.bz2
tar -xjf pcre-8.02.tar.bz2

cd pcre-8.02
./configure

if ! [ -f pcre_chartables.c ]
then
	cp pcre_chartables.c.dist pcre_chartables.c
fi

cd ..

rm pcre-8.02.tar.bz2
