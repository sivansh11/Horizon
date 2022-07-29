import os
import platform


# use your path for glslc here
# example shader_compiler = 'my_path_to_glslc'
shader_compiler = ''
if platform.system() == 'Windows' and shader_compiler == '':
    shader_compiler = 'C:\\programming\\shaderc\\bin\\glslc'
if platform.system() == 'Linux' and shader_compiler == '':
    shader_compiler = 'glslc'


if not os.path.exists("build"):
    print("Please run cmake!")
    quit()

if not os.path.exists("build/shaders"):
    os.mkdir("build/shaders")

targets = os.listdir("shaders")

for target in targets:
    print(shader_compiler + ' shaders/' + target + ' -o ' + 'build/shaders/' + target + '.spv')
    os.system(shader_compiler + ' shaders/' + target + ' -o ' + 'build/shaders/' + target + '.spv')
