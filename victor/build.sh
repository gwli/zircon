TOP=`pwd`/..

cd $TOP
#sudo true
#sudo apt-get install -y texinfo libglib2.0-dev autoconf libtool bison libsdl-dev build-essential
# ./scripts/download-prebuilt;
make clean
make -j32 USE_CLANG=true QUIET=0 x64 2>&1 |  tee build.log
