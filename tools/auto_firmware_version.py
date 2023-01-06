import subprocess
import json

Import("env")

def get_firmware_specifier_build_flag():    
    build_version = subprocess.run(["git", "describe", "--tags"], stdout=subprocess.PIPE, text=True).stdout.strip().strip('"')
    commit_date = subprocess.run(["git", "log", "-1", '--format="%ad"', "--date=short"], stdout=subprocess.PIPE, text=True).stdout.strip().strip('"')

    build_flag = "-D AUTO_FW_VERSION=\\\"" + build_version + "\\\" -D AUTO_FW_DATE=\\\"" + commit_date + "\\\""
    print ("Firmware revision:    " + build_version)
    print ("Firmware commit date: " + commit_date)

    with open("current-version.json", "w") as outfile:
        outfile.write(json.dumps({
            "revision": build_version,
            "date": commit_date
        }, indent=4))

    return (build_flag)

env.Append(
    BUILD_FLAGS=[get_firmware_specifier_build_flag()]
)
