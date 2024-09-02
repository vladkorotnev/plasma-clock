from os.path import isfile
Import("env")

if isfile("./lib/nonfree-aquestalk/libaquestalk.a"):
  print("Found a copy of AquesTalk")
  env.Append(BUILD_FLAGS=["-LD:lib/nonfree-aquestalk", "-laquestalk", "-DLIBAQUESTALK_FOUND"])
else:
  print("No AquesTalk library found")