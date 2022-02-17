> For this is how God loved the world:  
he gave his only Son, so that everyone  
who believes in him may not perish  
but may have eternal life.  
  \
John 3:16

## Geometry tool: assimp2bams

The command line tool __assimp2bams__ generates the format (Binary Asilva Mesh Scene).

The input is any Assimp supported 3D model format with or without animations (.obj, .blend, .fbx, .gltf, etc...).

The parameters to the command line tool:

* __-a:__ Export only animations
* __-gs:__ Export geometry and scene

If you don't pass any parameter to the tool, it will export all geometry information (animations, geomtry and scene).

Example:

```bash
# Generates a BAMS model file with the animations, 
# geometry and scene called full_export.bams
assimp2bams full_export.fbx

# Generates a BAMS model file with the geometry 
# and scene called geo_scene.bams
assimp2bams -gs geo_scene.fbx

# Generates a BAMS model file with only the animations
assimp2bams -a anim_only.fbx
```

## Related Links

https://github.com/A-Ribeiro/aRibeiroCore

https://github.com/A-Ribeiro/aRibeiroPlatform

https://github.com/A-Ribeiro/aRibeiroData

https://github.com/A-Ribeiro/aRibeiroWrappers

https://github.com/A-Ribeiro/aRibeiroTests

## Tools

https://github.com/A-Ribeiro/assimp2bams

https://github.com/A-Ribeiro/font2bitmap

## Authors

***Alessandro Ribeiro da Silva*** obtained his Bachelor's degree in Computer Science from Pontifical Catholic 
University of Minas Gerais and a Master's degree in Computer Science from the Federal University of Minas Gerais, 
in 2005 and 2008 respectively. He taught at PUC and UFMG as a substitute/assistant professor in the courses 
of Digital Arts, Computer Science, Computer Engineering and Digital Games. He have work experience with interactive
software. He worked with OpenGL, post-processing, out-of-core rendering, Unity3D and game consoles. Today 
he work with freelance projects related to Computer Graphics, Virtual Reality, Augmented Reality, WebGL, web server 
and mobile apps (andoid/iOS).

More information on: https://alessandroribeiro.thegeneralsolution.com/
