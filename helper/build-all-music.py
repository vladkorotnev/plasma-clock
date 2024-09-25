Import("env")
env.Execute("$PYTHONEXE -m pip install pyelftools")

import os

OUTDIR = "./data/music"
ELF2POMF = "./helper/elf2pomf.py"

def post_program_action(source, target, env):
    global OUTDIR, ELF2POMF
    music_path = os.path.join(str(target[0].get_dir()), "src", "music")
    print("Music path", music_path, "->", OUTDIR)

    for filename in os.listdir(OUTDIR):
        if filename.endswith(".pomf"):
            os.remove(os.path.join(OUTDIR, filename))

    for filename in os.listdir(music_path):
        if filename.endswith(".o"):
            filepath = os.path.join(music_path, filename)
            filename_base = os.path.basename(filename).split('.')[0]
            env.Execute(f"$PYTHONEXE {ELF2POMF} {filepath} {OUTDIR}/{filename_base}.pomf")

env.AddPostAction("$PROGPATH", post_program_action)