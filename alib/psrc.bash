# simple minded script to package the source & build stuff

# remove any existing
rm -rf src_package/*

mkdir src_package/src
cp ../../src/a_* src_package/src

mkdir src_package/inc
cp ../../inc/a_* src_package/inc

mkdir src_package/expat
cp ../../expat/* src_package/expat

mkdir src_package/build

mkdir src_package/build/gcc
cp ../gcc/Makefile src_package/build/gcc

mkdir src_package/build/vc
cp ../vc/vc.d* src_package/build/vc
cp ../vc/vc.mak src_package/build/vc

mkdir src_package/lib



