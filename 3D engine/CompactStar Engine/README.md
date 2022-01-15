<b><u>CompactStar engine</u></b>

The CompactStar engine is a tiny game engine I'm using for my own purposes. On the Mobile C Compiler it serves me as base to create small game prototypes.

<b>Here are the actual supported features</b>
- Simple shapes creation (surface, box, sphere, cylinder, disk, ring and spiral)
- Quake I (.mdl), DirectX (.x, partially), Collada (.dae, partially) and WaveFront (.obj, partially) models
- Animations for Quake I (.mdl), DirectX (.x) and Collada (.dae) models
- Skybox
- Landscape generation
- Transparency
- Bump mapping
- Full-scene antialiasing and post-processing effects
- Collision detection (partially, geometry, ground and mouse collision)
- Particles system (early stage, see the Weather demo and Spaceship game demo)
- Physics (early stage, see the Wild Soccer demo)
- Artificial intelligence (early stage, see the Bot demo)
- Sound and music
- Cross-platform SDK written in C
- Objective-C Metal renderer for OSX/iOS (experimental)

<b>Here are some screenshots of several projects I realized with this engine (all are available as demo)</b>

![Screenshot](Common/Images/Screenshots/Demos.png?raw=true "Screenshot")

<b>Supported compiler versions and devices</b>

The CompactStar Engine is fully supported by the Mobile C Compiler since the version 2.4.1. Earlier versions may not work well, or not work at all.

Also, provided demos are targetting devices since iPhone 6. Earlier devices may run the demo very slowly, or not at all.

<b>About the Collada format</b>

The Collada (.dae) support is partial and was planned to support very simple models. For that reason several restrictions should be considered:
- Only triangles are supported in geometry, for that reason the Triangulate option should be activated in exporter. See e.g. the following Blender exporter screenshot:
![Screenshot](Common/Images/Screenshots/BlenderExporterGeom.png?raw=true "Screenshot")
- The skeleton should be as simple as possible, complex skeletons may not be well supported. Avoid also to create more than 1 skeleton
- Avoid to create many textures, if possible, create only one simple RGB texture
- Only matrices transformations are supported for animations. Also only linear interpolations are supported. See e.g the following Blender exporter screenshot:
![Screenshot](Common/Images/Screenshots/BlenderExporterAnim.png?raw=true "Screenshot")
- An example of well supported model is provided in resources (see cat.blend file)
<b>NOTE</b> this cat model is a free 3d model from Sketchfab, available here: https://sketchfab.com/3d-models/lowpoly-cat-rig-run-animation-c36df576c9ae4ed28e89069b1a2f427a

<b>To go further</b>

An advanced version of this SDK is also available, which contains more demos and advanced tools or prototypes, like an early stage 2D level designer.

You may find this project here: https://github.com/Jeanmilost/CompactStar

<b>Copyright</b>

I decided to share this code freely, don't hesitate to use it if you think it useful for your purposes. Any collaboration on this project is welcome.
