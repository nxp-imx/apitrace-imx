# Build for i.MX6 #
## Yocto ##

    export CROSSTOOLCHAIN=...
    export CMAKE_SYSROOT=...
    cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/arm-gnueabi.toolchain.cmake -f CMakeLists.txt -DCMAKE_INSTALL_PREFIX:PATH=<path>
    make && make install


## Android ##

    cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/android.toolchain.cmake -f CMakeLists.txt -DCMAKE_INSTALL_PREFIX:PATH=<path> -DANDROID_NATIVE_API_LEVEL=android-19 -DANDROID_NDK=<ndk tool>

KitKat uses sdk level 19; Lollipop uses 21:
    make && make install


# Build for x86 #

    cmake -f CMakeLists.txt -DCMAKE_INSTALL_PREFIX:PATH=<path>
    make && make install

# Tips #
## Switch cross build and native build ##

    make clean
    find . -name CMakeFiles -exec rm -rf "{}" \;
    find . -name CMakeCache.txt -exec rm "{}" \;


## To enable qapitrace ##

You must install qt4 development packages:

    sudo apt-get install qt4-default qt4-qmake qt4-dev-tools libqt4-dev libqt4-opengl-dev automoc


## Fix empty AR in link.txt created by cmake ##

For cross build, macro CMAKE_AR may not work. Use following command to replace the empty AR command with correct value:

    find . -name "link.txt" -exec grep -l -e "\"\" cr" "{}" \; | xargs sed -i 's#\"\"#'$CROSSTOOLCHAIN'-ar#'
