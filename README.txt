Toy OpenGL program drawing line segments. Also some 3D mesh generation.

Builds on Linux with Xlib. On Windows, Win32 is used.

For development, my separate project "glsl-processor" is needed to generate the
shader interfaces. You can use git submodules to avoid dependency headaches
here. Full build instructions for Linux:

    git clone --recursive https://github.com/jstimpfle/segments
    cd segments
    make
