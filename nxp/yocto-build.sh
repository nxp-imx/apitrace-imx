#! /bin/bash

set -x
set -e

CWD="`dirname $(realpath $0)`"
SCRIPT_DIR="$(realpath $CWD)"

###     MODIFY THESE TO FIT YOUR SETUP
DEST_DIR=/nfsroot/apitrace10
WAFFLE_DIR=$SCRIPT_DIR/../../waffle
APITRACE_DIR=$SCRIPT_DIR/..
SDK_DIR=/work/adrian/workspace/Yocto/internal_xwayland_glibc-x86-64_gui_aarch64_4.19_warrior
##############################################


export VERBOSE=1
BUILD_DIR=__build_aarch64
BUILD_TYPE=Debug

build_waffle()
{
      pushd $WAFFLE_DIR
      mkdir -p $BUILD_DIR

      export SDKTARGETSYSROOT="$SDK_DIR/sysroots/aarch64-poky-linux"
      export PKG_CONFIG_SYSROOT_DIR="$SDKTARGETSYSROOT"
      export PKG_CONFIG_PATH="$DEST_DIR/lib/pkgconfig:$SDKTARGETSYSROOT/usr/lib/pkgconfig:$SDKTARGETSYSROOT/usr/share/pkgconfig"
      meson setup \
            --cross-file $SCRIPT_DIR/yocto-sdk-meson.ini \
            -Dglx=disabled \
            -Dwayland=enabled \
            -Dgbm=enabled \
            -Dsurfaceless_egl=disabled \
            -Dprefix=/usr \
            $BUILD_DIR
      ninja -v -C $BUILD_DIR
      popd
}

install_waffle()
{
      pushd $WAFFLE_DIR
      pushd $BUILD_DIR
      DESTDIR="$SDK_DIR/sysroots/aarch64-poky-linux" ninja install
      popd
      popd
}

clean_waffle()
{
      pushd $WAFFLE_DIR
      rm -rf $BUILD_DIR
      popd
}

clean_apitrace()
{
      pushd $APITRACE_DIR
      rm -rf $BUILD_DIR
      popd
}

build_apitrace()
{
      pushd $APITRACE_DIR
      mkdir -p $BUILD_DIR

      export PKG_CONFIG_PATH="$DEST_DIR/lib/pkgconfig:$SDKTARGETSYSROOT/usr/lib/pkgconfig:$SDKTARGETSYSROOT/usr/share/pkgconfig"
      cmake \
            -G 'Ninja' \
            -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
            -DENABLE_EGL=ON \
            -DENABLE_WAFFLE=ON \
            -DENABLE_VIVANTE=ON \
            -DENABLE_GUI=OFF \
            -DCMAKE_TOOLCHAIN_FILE=${SCRIPT_DIR}/OEToolchainConfig.cmake \
            -B $BUILD_DIR
      ninja -v -C $BUILD_DIR
      popd
}

install_apitrace()
{
      pushd $APITRACE_DIR
      pushd $BUILD_DIR
      DESTDIR="$DEST_DIR" ninja install
      popd
      popd
}

rm -rf $DEST_DIR


clean_waffle
build_waffle
install_waffle


source $SDK_DIR/environment-setup-aarch64-poky-linux

clean_apitrace
build_apitrace
install_apitrace


###########################
rm -rf $DEST_DIR/usr/local/bin/wrappers
mv     $DEST_DIR/usr/local/lib/aarch64-linux-gnu/apitrace/wrappers \
       $DEST_DIR/usr/local/bin/wrappers
