import os

shader_compiler = 'C:\\programming\\shaderc\\bin\\glslc'

if not os.path.exists("build"):
    print("Please run cmake!")
    quit()

if not os.path.exists("build/shaders"):
    os.mkdir("build/shaders")

targets = os.listdir("shaders")

for target in targets:
    print(shader_compiler + ' shaders/' + target + ' -o ' + 'build/shaders/' + target + '.spv')
    os.system(shader_compiler + ' shaders/' + target + ' -o ' + 'build/shaders/' + target + '.spv')
