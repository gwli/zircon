TOP=`pwd`/..

cd $TOP

function test1() {
sudo true
sudo apt-get install -y texinfo libglib2.0-dev autoconf libtool bison libsdl-dev build-essential
./scripts/download-prebuilt;
make clean
make -j32 USE_CLANG=true QUIET=0 x64 2>&1 |  tee build.log
}

function test2() {
 time ./scripts/build-zircon -C -v -a x64 2>&1 | tee build.log
}

test2

