# VortexEditor

Editor to play around with Vortex2D. 

See https://github.com/mmaldacker/Vortex2D

<p align="middle">
  <img src="https://github.com/mmaldacker/VortexEditor/raw/master/VortexEditor.png"/> 
</p>

<div align="center">
  <a href="https://www.youtube.com/watch?v=4xzALHymYmw"><img src="https://img.youtube.com/vi/4xzALHymYmw/0.jpg"></a>
</div>

# Building

The usual cmake command (dependencies are downloaded by cmake):

```
mkdir build && cd build
cmake ..
make -j 4
```

Platforms supported:

* Linux
* Window

See Vortex2D for more details on how to build.

# Manual

## Drawing shape

Right click to open the contextual window and select shape (rectangle or circle).

* Left click and drag to create rectangle/circle.
* Once created, left click and drag, to move around.
* Ctrl + left click to rotate.

## Adding water

Right click to open the contextual window and select fluid.

* Left click to add water.
* Left click and drag to add force.
* Ctrl + left click and drag to add more water.

## Objects

Right click on a shape to open the contextual window:

* Static/Dynamic object.
* Delete object.
* Clean to remove all objects.