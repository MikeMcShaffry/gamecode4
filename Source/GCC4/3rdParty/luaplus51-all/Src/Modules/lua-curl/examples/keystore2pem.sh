#!/bin/bash

# $Id: keystore2pem.sh,v 1.2 2007/09/20 20:39:13 juergen Exp $

# Convert JDK/JREs cacerts keystore to PEM using keytool/openssl
# Author: Juergen Hoetzel



PASSWORD=changeit		# default keystore password

if [[ -z $JAVA_HOME ]]; then 
    echo JAVA_HOME no set >&2
    exit 1
fi

KEYSTORE=$JAVA_HOME/jre/lib/security/cacerts

if [[ ! -r $KEYSTORE ]]; then
    echo Cannot read keystore $KEYSTORE >&2
    exit 1
fi
aliases=`keytool -list -v -storepass $PASSWORD -keystore $KEYSTORE|grep Alias|cut -d : -f 2`
for alias in ${aliases[@]}; do
    keytool -export -storepass $PASSWORD -keystore $KEYSTORE -alias $alias|
    openssl x509 -outform pem -inform derc
done
