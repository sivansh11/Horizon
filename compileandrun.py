import os

os.system("cmake -S. -Bbuild")

os.system("cd build && make")

import shadercompiler

os.system("cd build && ./horizon")