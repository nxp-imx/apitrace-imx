Sync with mainline project
==========================

By mainline, I refer to jrfonseca's repo found at: https://github.com/apitrace/apitrace.git

Add mainline repo:

```sh
git remote add mainline-origin https://github.com/apitrace/apitrace.git
git fetch mainline-origin
```


List mainline tags:

```sh
git ls-remote --tags origin
```

Make a branch out of a mainline tag listed at the previous step.

```sh
git checkout -b imx_10 tags/10.0
```


Build with Yocto SDK
====================

Download waffle and apitrace and place them on the same directory level.
Change the variables at the top of nxp/yocto-build.sh and nxp/yocto-sdk-meson.ini
to suit your setup.


Build with QNX SDK
==================
