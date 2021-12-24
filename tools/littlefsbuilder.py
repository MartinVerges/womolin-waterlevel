#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import stat
from sys import platform

Import("env")

if platform == "linux" or platform == "linux2":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs_linux"
    print("Replace MKSPIFFSTOOL with " + file)
    st = os.stat(file)
    os.chmod(file, st.st_mode | stat.S_IEXEC)
elif platform == "darwin":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs_darwin"
    print("Replace MKSPIFFSTOOL with " + file)
    st = os.stat(file)
    os.chmod(file, st.st_mode | stat.S_IEXEC)
elif platform == "win32":
    file = env.get("PROJECT_DIR") + "/tools/mklittlefs_win.exe"
    print("Replace MKSPIFFSTOOL with " + file)

env.Replace(MKSPIFFSTOOL=file)
