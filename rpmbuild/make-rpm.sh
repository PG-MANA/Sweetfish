#!/bin/sh
APP_NAME_DEFINE=(`cat build.spec | grep APP_NAME | grep %define`)
APP_VERSION_DEFINE=(`cat build.spec | grep APP_VERSION | grep %define`)
APP_NAME=${APP_NAME_DEFINE[2]}
APP_VERSION=${APP_VERSION_DEFINE[2]}
COPY_PATH=~/rpmbuild/SOURCES/

mkdir ${APP_NAME}-${APP_VERSION}
cd ${APP_NAME}-${APP_VERSION}
ln -s  ../${APP_NAME}.desktop ../../CMakeLists.txt ../../src ./
cd ../
tar zchvf ${APP_NAME}-${APP_VERSION}.tar.gz ${APP_NAME}-${APP_VERSION} 
rm -rf  ${APP_NAME}-${APP_VERSION}
if [ ! -e ${COPY_PATH} ]; then
rpmbuild
fi
mv ${APP_NAME}-${APP_VERSION}.tar.gz $COPY_PATH
rpmbuild -bb build.spec $*
