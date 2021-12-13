Import("env")
print("Replace MKSPIFFSTOOL with mklittlefs.exe")
env.Replace( MKSPIFFSTOOL=env.get("PROJECT_DIR") + '/tools/mklittlefs' )