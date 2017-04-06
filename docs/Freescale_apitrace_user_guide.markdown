# Freescale apitrace release note

## Overview

Freescale apitrace is a set of tools enhanced from open source project apitrace.
Freescale version adds support to Freescale i.MX6 chipsets which use Vivante GPU IP.


## Purpose

This tool is to provide the gpu application and driver developers a way to look
inside the application with source code. It also provides a way to port an
application to another platform with source requirement. Comparing with
Vivante’s similar tool vTracer/vPlayer, Freescale apitrace provides some new
features:

```
Feature                     Vivante Apitrace   Comment
---------------------------+-------+-------+-------------------------------------.
Easy to use                | N     | Y     | Vivante’s tools have Too many       |
                           |       |       | settings to remember, and user must |
                           |       |       | make sure tool version matches gpu  |
                           |       |       | driver version. There’s a strong    |
                           |       |       | dependency between tool and gpu     |
                           |       |       | driver.                             |
---------------------------+-------+-------+-------------------------------------+
License                    | N     | Y     | vPlayer/vTracer: closed source      |
control                    |       |       | Apitrace: BSD                       |
-------+-+-----------------+-------+-------+-------------------------------------+
Trace  |O| Linux           | Y     | Y     |                                     |
       |S| Android         | Y     | Y     |                                     |
       |-+-----------------+-------+-------+-------------------------------------+
       |A| OpenGL          | N     | Y     |                                     |
       |P| GLES1           | Y     | Y     |                                     |
       |I| GLES2           | Y     | Y     |                                     |
       | | GLES3           | Y     | Y     | With my patch to support ES 3.0     |
       | | OpenVG          | ???   | ???   |                                     |
-------+-+-----------------+-------+-------+-------------------------------------+
Replay |O| Linux           | Y     | Y     |                                     |
       |S| Android         | Y     | ???   |                                     |
       | | Windows         | Y     | Y     |                                     |
       |-+-----------------+-------+-------+-------------------------------------+
       |Frame dump         | Y     | Y     |                                     |
       |Frame range&repeat | Y     | Y     |                                     |
       |Insert error check | Y     | Y     |                                     |
       |Insert glFinish    | Y     | N     |                                     |
       |Dump per-call state| N     | Y     |                                     |
-------+-------------------+-------+-------+-------------------------------------+
Post   | View framebuffer  | N     | Y     |                                     |
process| View textures     | N     | Y     |                                     |
       | Editing           | N     | Y     |                                     |
       | Performance Mon   | N     | Y     |                                     |
       | Encode to video   | N     | Y     |                                     |
-------+-------------------+-------+-------+-------------------------------------'
```

## Release History
```
Version  Date
.-------+---------.
| 0.1   | 2014/12 |
'-------+---------'
```


## Known Issues

```
MGS-310 APITRACE: Will print "/usr/lib/libstdc++.so.6: _ZNSo5writeEPKci+0x4f" using udisk to store trace files
MGS-309 APITRACE: retrace "tutorial3" is different with trace it
MGS-308 APITRACE: Should support to create children dir
```
