OgreBall Mesh Scripting
========================

## Plane Meshes

planemesh <name>:

* dir <x> <y> <z> <offset>
* size <width> <height>
* up <x> <y> <z>    # up direction for texture
* segments <x> <y>
* texRepeat <x> <y>


## Extruded Meshes

Path: 3-dimensional line describing the trail of the mesh.
Shape: 2-dimensional shape that will be extruded (dragged) along the path of the mesh.
Track: Describes how textures should be mapped onto the mesh.

extrudedmesh <name>:

* scale <x> <y> <z>
* tiles <utiles> <vtiles>
* path:
  * type <typename = default: straight lines | choices: previous, previous-combined, catmullSpline, cubicHermiteSpline, roundedCornerSpline, bezierCurve>
  * scale <x> <y> <z>
  * translate <x> <y> <z>
  * segments <numSegments>
  * close <true | false = false>  # Connect first and last points on path
  * radius <radius>  # Only use if roundedCornerSpline
  * points:
    * p <x> <y> <z>
    ...
... (i.e. multiple paths combined by appending)

* shape:
  * type <typename = default: straight lines | choices: previous, previous-combined, catmullSpline, cubicHermiteSpline, roundedCornerSpline, kbSpline, bezierCurve, rectangle, circle, ellipse, triangle>
  * scale <x> <y>
  * rotate <degrees>
  * translate <x> <y>
  * mirror <x> <y>  # Around a point
  * mirroraxis <x> <y>  # Around an axis/vector
  * segments <numSegments>  # only use if spline, curve, circle, or ellipse (not rectangle or triangle)
  * close <true | false = false>  # Connect first and last points on path; only for splines and curves
  * radius <radius>  # only use if roundedCornerSpline or circle
  * width <w>   # Only use if rectangle
  * height <h>  # Only use if rectangle
  * length <l> OR length <alength> <blength> <clength>  # Only use if triangle
  * points:
    * p <x> <y>
    ...
  * combine <typename = default: append | choices: union, intersection, difference>  # performs combine op with the shape defined up to that point
  * outside <left | right = right>
  * thicken <thickness>  # Take line shape and add thickness to it (i.e. like bubble letters)
... (i.e. multiple shapes combined by combine functions)

* track
  * addressingMode <typename = default: absolute linear; choices: relative, point>
  * keyframes:
    * keyframe <k1> <k2>
    ...

## Advanced Features

String together paths by placing them one after another

Combine shapes by placing them one after another (can change how they combine using the 'combine' option)

Duplicate paths or strings by using type=previous (duplicate last path created) or type=previous-combined (duplicate the entirety of the path created up until that point)

Most options are NOT needed, but if you omit an option that IS needed, the program will crash with a segfault...sorry