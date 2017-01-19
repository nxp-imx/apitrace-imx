#! /bin/bash

# tool to auto-test apitrace/retrace on linux-yocto
# this tool support x11/fb/dfb/wayland backends

if [ $# != 2 ]; then
    echo "Usage: $0 <backend> <tracefile path>"
    echo "    backend: can be one of fb/dfb/wayland/x11"
    echo "    tracefile path: where to put trace file"
    exit -1
fi

BACKEND=$1
TRACEFILE_PATH="$(realpath $2)"
MAXDURATION="10s" # duration for every case. depending on available disk space

EGL11_CASES="angeles tritex OGLESCoverflow tutorial1 tutorial2 tutorial3 tutorial4 tutorial5 tutorial6 tutorial7"
EGL20_CASES="es2gears OGLES2Coverflow OGLES2Fog OGLES2Texturing scene_FBO glmark2-es2 tutorial1_es20 tutorial2_es20 tutorial3_es20 tutorial4_es20 tutorial5_es20 tutorial6_es20 tutorial7_es20"
EGL30_CASES="OGLES3PhantomMask OGLES3DeferredShading OGLES3RenderToTexture"
#GLX_CASES="glxgears glmark2 glxs"
GLX_CASES="glxgears glxs"

kill_app()
{
    APP=$1
    # Need truncate app name, otherwise pkill will fail on yocto
    pkill -x "${APP:0:15}"
}

run_eglcases()
{
    for tcase in $1
    do
        CURPATH="$(realpath .)"
        echo "--------------------------------------------------------"
	echo "Start apitrace"
        echo "--------------------------------------------------------"
        # for tutorial cases, need go to that directory to run
	if [ "${tcase:0:8}" == "tutorial" ]; then
            cd /opt/viv_samples/vdk
	fi
        # start test
	apitrace trace --output=$TRACEFILE_PATH/$tcase.trace --api=egl $tcase &
        sleep $MAXDURATION
        # end test
	kill_app $tcase
        cd $CURPATH # go back to original path

	echo "--------------------------------------------------------"
	echo "Start retrace"
        echo "--------------------------------------------------------"
	sleep 1s
        eglretrace $TRACEFILE_PATH/$tcase.trace
    done
}

run_glxcases()
{
    for tcase in $1
    do
        CURPATH="$(realpath .)"
	echo "--------------------------------------------------------"
	echo "Start apitrace"
        echo "--------------------------------------------------------"

	if [ "$tcase" == "glxs" ]; then
            cd /share/X11Tests/yocto1.6rel/bin/GLXS
	    tcase="./$tcase"
	fi

        # start test
	apitrace trace --output=$TRACEFILE_PATH/$tcase.trace --api=gl $tcase &
        sleep $MAXDURATION
        # end test
	kill_app $tcase
        cd $CURPATH # go back to original path

	echo "--------------------------------------------------------"
	echo "Start retrace"
        echo "--------------------------------------------------------"
	sleep 1s
        glretrace $TRACEFILE_PATH/$tcase.trace
    done
}

run_cases()
{
    export PATH=$PATH:/share/X11Tests/yocto1.6rel/bin:/opt/viv_samples/vdk
    export PATH=$PATH:/share/X11Tests/yocto1.6rel/bin/PowerVR3.1/Advanced/:/share/X11Tests/yocto1.6rel/bin/PowerVR3.1/Intermediate/:/share/X11Tests/yocto1.6rel/bin/PowerVR3.1/Beginner/
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/share/X11Tests/yocto1.6rel/lib
    if [ "$BACKEND" == "x11" ]; then
    export DISPLAY=:0
    fi
    mkdir -p $TRACEFILE_PATH
    rm $TRACEFILE_PATH/*.trace

    run_eglcases "$EGL11_CASES"
    run_eglcases "$EGL20_CASES"
    run_eglcases "$EGL30_CASES"
    if [ "$BACKEND" == "x11" ]; then
    run_glxcases "$GLX_CASES"
    fi
}

date

run_cases

echo "--------------------------------------------------------"
echo "Test finished"
date
echo "--------------------------------------------------------"
