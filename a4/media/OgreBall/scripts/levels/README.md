OgreBall Level Scripting
==========================

## Level Setup

start:

* skybox <matName> [ <distance> ]
* skydome <matName> [ <curvature> <tiling> <distance> ]
* camera:
  * pos <x> <y> <z>
  * lookAt <x> <y> <z>
* player[1-4]:
  * pos <x> <y> <z>
  * rot <x> <y> <z>
* waypoints:
  * waypoint:
    * pos <x> <y> <z>
    * rot <x> <y> <z>

lights:

* ambient <r> <g> <b>
* <lightname>:
  * type < typename = default: point | choices: directional, spotlight >
  * diffuse <r> <g> <b>
  * specular <r> <g> <b>
  * direction <x> <y> <z>  # for directional lights
  * range <r1> <r2>        # for spotlights

goal:

* pos:
  * start <x> <y> <z>
  * interp <timeFromLast> <x> <y> <z>
  ...

* rot:
  * start <yaw> <pitch> <roll>
  * interp <timeFromLast> <yaw> <pitch> <roll>
  ...

## Valid Objects

* plane
* extrudedObject
* collectible
* decorative

## Object Parameters

* meshname <meshName>
* material <matName>
* scale <x> <y> <z>  # This will NOT scale extruded mesh collision bodies; scale in the extrudedMesh script.
* mass <m = 0.0f>
* rest <r = 0.9f>
* kinematic <true | false = true>
* ambient <r> <g> <b>
* diffuse <r> <g> <b> <a>
* specular <r> <g> <b> <a>
* pos:
  * start <x> <y> <z>
  * interp <timeFromLast> <x> <y> <z>
  ...
* rot:
  * start <yaw> <pitch> <roll>
  * interp <timeFromLast> <yaw> <pitch> <roll>
  ...

## Advanced Features

Nest object definitions within other object definitions to create parent-child relations.