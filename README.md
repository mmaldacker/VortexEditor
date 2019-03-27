# VortexEditor

Editor to play around with Vortex2D. 

See https://github.com/mmaldacker/Vortex2D

<p align="middle">
  <img src="https://github.com/mmaldacker/VortexEditor/raw/master/VortexEditor.png"/> 
</p>

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

Select rectangle or circle from shape drawing.

* Left click and drag to create rectangle/circle.
* Once created, left click and drag, to move around.
* Ctrl + left click to rotate

## Adding water

* Right click to add water
* Right click and draw to add force

## Objects

* Select shape from drop down menu, choose static/dynamic type and click update.
* Select shape from drop down menu, delete to remove.
* Clean to remove all objects.