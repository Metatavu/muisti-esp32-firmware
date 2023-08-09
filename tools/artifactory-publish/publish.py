import requests
import sys
import os
import argparse
from urllib.parse import urlparse

#
# Push new firmware to the artifactory storage using API
#
def publish_firmware(
  source: str,
  repository_url: str,
  module: str,
  repository: str,
  organization: str,
  version: int,
  api_token: str
):
    firmware_name = module + "-" + str(version) + ".bin"

    print("Uploading {0} to Artifactory. Version: {1}".format(firmware_name, str(version)))

    url = "/".join([repository_url, "artifactory", repository, organization, module, firmware_name ])

    headers = {
        "Content-type": "application/octet-stream",
        "Authorization": "Bearer " + api_token
    }

    r = None
    try:
        r = requests.put(url, data=open(source, "rb"), headers=headers)
        r.raise_for_status()
    except requests.exceptions.RequestException as e:
        sys.stderr.write("Failed to submit package: %s\n" %
                         ("%s\n%s" % (r.status_code, r.text) if r else str(e)))
        env.Exit(1)

    print("The firmware has been successfuly published")

def is_file(value):
    if not os.path.isfile(value):
        raise argparse.ArgumentTypeError(f"{value} is not a valid file path")
    return value

def is_number(value):
    try:
        return int(value)
    except ValueError:
        raise argparse.ArgumentTypeError(f"{value} is not a valid number")

def is_url(value):
    result = urlparse(value)
    if all([result.scheme, result.netloc]):
        return value
    else:
        raise argparse.ArgumentTypeError(f"{value} is not a valid URL")

argument_parser = argparse.ArgumentParser(description="Receive required command-line arguments.")

argument_parser.add_argument("--source", type=is_file, required=True, help="Path to the source file.")
argument_parser.add_argument("--version", type=is_number, required=True, help="Version number.")
argument_parser.add_argument("--repository", type=str, required=True, help="Repository URL.")
argument_parser.add_argument("--repository_url", type=is_url, required=True, help="Repository URL.")
argument_parser.add_argument("--module", type=str, required=True, help="Module name.")
argument_parser.add_argument("--organization", type=str, required=True, help="Organization name.")
argument_parser.add_argument("--api_token", type=str, required=True, help="API Token.")

args = argument_parser.parse_args()

publish_firmware(
    source=args.source,
    version=args.version,
    repository_url=args.repository_url,
    module=args.module,
    repository=args.repository,
    organization=args.organization,
    api_token=args.api_token
)