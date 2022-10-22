# horizon


## Vulkan SDK
download vulkan sdk from
https://www.lunarg.com/vulkan-sdk/

NOTE: you need the vulkan sdk inorder to run this program

## Build instructions
in root dir 
`cmake -S. -Bbuild`

go to the build dir made by cmake 
`cd build`

run make 
`make`


## Runing Horizon  
Before you can run horizon, you have to compile the shaders
First return to the root directory with
`cd ..`

To compile the shaders either run compile_shaders.sh or compile_shader.bat
`compile_shader.sh` or `compile_shader.bat`

NOTE: This is done thru a python script (shadercompiler.py) so you must have python installed
If the script errors out, then open shadercompiler.py and use your location of glslc instead 

To run Horizon
`cd build && ./horizon`


## The Future
- python scripting
- multiple graphics backend 
- model rigging support
- editor
