# StarFox

A StarFox-like game written in C++.

I put this project together mostly because I wanted to test out some ideas I had for building a Unity3D-inspired object/component model and scene graph.

![Screenshot](https://raw.githubusercontent.com/wiki/amaiorano/StarFox/images/StarFox-1.png)

# Building the Code

This project is developed on Windows, but should be easy enough to port to other platforms. This would mainly consist of implementing the platform-specific files in gsgamelib/src/gs/platform.

Assuming you're on Windows:

Install [Visual Studio](https://www.visualstudio.com) (2013 or later)

Install [Autodesk FDB SDK](http://www.autodesk.com/products/fbx/overview)

```
cd StarFox
mkdir build & cd build
cmake ..
```

Open starfox.sln and build the solution.

To run the game, set starfoxgame as the startup project in Visual Studio, and set the Debug Working Directory to point to ```Starfox/source/starfoxgame```.

Build the INSTALL project to have it install the game and data files to ```StarFox/bin```.
