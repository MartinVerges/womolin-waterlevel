#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import stat
from sys import platform, path

print( path )

Import("env")

if platform == "linux" or platform == "linux2":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs_linux"
    print("Replace MKFSTOOL with " + file)
    st = os.stat(file)
    os.chmod(file, st.st_mode | stat.S_IEXEC)
elif platform == "darwin":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs_darwin"
    print("Replace MKFSTOOL with " + file)
    st = os.stat(file)
    os.chmod(file, st.st_mode | stat.S_IEXEC)
elif platform == "win32":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs_win.exe"
    print("Replace MKFSTOOL with " + file)

env.Replace(MKFSTOOL=file)
