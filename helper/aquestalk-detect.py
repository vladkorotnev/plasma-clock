from os.path import isfile, join, abspath
from os import getcwd
Import("env")

DIR_PATH=abspath(join(getcwd(), "lib/nonfree-aquestalk")).replace("\\", "/")
LIB_PATH=join(DIR_PATH, "libaquestalk.a")

if isfile(LIB_PATH):
  print("Found a copy of AquesTalk:", LIB_PATH)
  env.Append(BUILD_FLAGS=["-L"+DIR_PATH, "-laquestalk", "-DLIBAQUESTALK_FOUND"])
else:
  print("No AquesTalk library found")