import subprocess

Import("env")

def get_firmware_specifier_build_flag():    
    build_version = subprocess.run(["git", "describe"], stdout=subprocess.PIPE, text=True).stdout.strip()
    commit_date = subprocess.run(["git", "log", "-1", '--format="%ad"', "--date=short"], stdout=subprocess.PIPE, text=True).stdout.strip()

    build_flag = "-D AUTO_FW_VERSION=\\\"" + build_version + "\\\" -D AUTO_FW_DATE=\\\"" + commit_date + "\\\""
    print ("Firmware revision:    " + build_version)
    print ("Firmware commit date: " + commit_date)
    return (build_flag)

env.Append(
    BUILD_FLAGS=[get_firmware_specifier_build_flag()]
)
