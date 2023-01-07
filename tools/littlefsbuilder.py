#!/usr/bin/python
# -*- coding: utf-8 -*-

# Important to know:
# ===============================================================
# With this software I ship a pre build version of mklittlefs.
# If you want to compile your own executable, please refer to
#    https://github.com/earlephilhower/mklittlefs
#
# In addition, it's super important to know, that used the build
# parameter LFS_MAX_NAME=128 in order to support filenames from
# the svelte UI. You can compile it using:
#    make dist CPPFLAGS="-DLFS_NAME_MAX=128"
#
# This software and repo is not meant to be executed on anything
# else than an ESP32 microcontroller.
# ===============================================================

import os
import stat
import sys
from sys import platform, path

Import("env")

if not os.path.exists("tools/mklittlefs"):
    file =  env.get("PROJECT_DIR") + "/tools/build_littlefs_tool.sh"
    st = os.stat(file)
    os.chmod(file, st.st_mode | stat.S_IEXEC)
    
    os.chdir(env.get("PROJECT_DIR") + "/tools")
    env.Execute(file)
    os.chdir(env.get("PROJECT_DIR"))

if platform == "linux" or platform == "linux2":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs"
    print("Replace MKFSTOOL with " + file)
    st = os.stat(file)
    os.chmod(file, st.st_mode | stat.S_IEXEC)
    # Build the UI before buildfs
    #env.Execute("cd ui; npm install; npm run build; cd ..");
    print("[WARN] No automatic UI build for this platform", file=sys.stderr)

elif platform == "darwin":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs"
    print("Replace MKFSTOOL with " + file)
    st = os.stat(file)
    os.chmod(file, st.st_mode | stat.S_IEXEC)
    # Build the UI before buildfs
    #env.Execute("cd ui; npm install; npm run build; cd ..");
    print("[WARN] No automatic UI build for this platform", file=sys.stderr)

elif platform == "win32":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs.exe"
    print("Replace MKFSTOOL with " + file)

    # FIXME: Build the UI before buildfs
    print("[WARN] No automatic UI build for this platform", file=sys.stderr)

env.Replace(MKFSTOOL=file)
