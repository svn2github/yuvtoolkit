YUV Toolkit
=================================================
http://www.yuvtoolkit.com

YUV Toolkit is an Open Source, cross platform raw
yuv player and analyzer. It is currently in early
stage of development, and current development 
focus is on playback features.

Version History
=================================================

0.0.2
-----
* Major New Features
New video pipeline, more efficient playback and more reliable seeking.
Support for 720P 60FPS playback of 4 side-by-side videos using D3D renderer.
Support for MSE and PSNR objective measures and error map visualization.

* Minor New Features
Improved perception of start-up time (i.e. show window early).
Continue playback after seeking to a new frame using mouse.
Accelerating seeking when pressing down arrow keys.
HOME and END keys for beginning and end of video.
Improved algorithm for video layout (minimizing background).
New toolbar
Removed QPaint renderer

* Changes in YTS scripting API
Added new openFiles API to load several files at same time (more reliable):

yt.openFiles([
  '480p/Andrei_2_sony_hd_640x480_30fps.yuv',
  '480p/David-12-iSight-LowLight-640x480-15fps.yuv'
  ]);


0.0.1
-----
Initial Release
Support for Windows and OsX
Direct3D renderer for windows
OpenGL and QPaint based cross platform renderers