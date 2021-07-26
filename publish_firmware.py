import requests
import sys
from os import getenv
from os.path import basename

Import('env')

# from platformio import util
# project_config = util.load_project_config()
# artifactory_config = {k: v for k, v in project_config.items("artifactory")}
# version = project_config.get("common", "release_version")

try:
    import configparser
except ImportError:
    import ConfigParser as configparser
project_config = configparser.ConfigParser()
project_config.read("platformio.ini")
version = project_config.get("common", "release_version")
artifactory_config = {k: v for k, v in project_config.items("artifactory")}


def get_config(key):
    value = artifactory_config.get(key)
    if (value.startswith("${sysenv.")):
        env_name = value[9:-1]
        value = getenv(env_name)

    return value

#
# Push new firmware to the artifactory storage using API
#
def publish_firmware(source, target, env):
    firmware_path = str(source[0])
    firmware_name = get_config("module") + "-" + version + ".bin"

    print("Uploading {0} to Artifactory. Version: {1}".format(
        firmware_name, version))

    url = "/".join([
        "https://metatavu.jfrog.io", "artifactory",
        get_config("repository"),
        get_config("organization"), get_config("module"), firmware_name
    ])

    headers = {
        "Content-type": "application/octet-stream",
        "Authorization": "Bearer " + get_config("api_token")
    }

    r = None
    try:
        r = requests.put(url,
                         data=open(firmware_path, "rb"),
                         headers=headers)
        r.raise_for_status()
    except requests.exceptions.RequestException as e:
        sys.stderr.write("Failed to submit package: %s\n" %
                         ("%s\n%s" % (r.status_code, r.text) if r else str(e)))
        env.Exit(1)

    print("The firmware has been successfuly published")

# Custom upload command and program name
env.Replace(PROGNAME="firmware_v_%s" % version, UPLOADCMD=publish_firmware)
