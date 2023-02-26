#!/usr/bin/env python3

import argparse
import json
import csv
import os
import sys

manifestFile = os.path.normpath(os.path.dirname(__file__) + '/webinstaller-manifest-template.json')
if not os.path.isfile(manifestFile):
    sys.exit("[ERROR] Unable to open %s" % manifestFile)

currentVersionFile = os.path.normpath(os.path.dirname(__file__) + '/../current-version.json')
if not os.path.isfile(currentVersionFile):
    sys.exit("[ERROR] Unable to open %s" % currentVersionFile)

with open(currentVersionFile) as versionFile:
  currentVersion = json.load(versionFile)

parser = argparse.ArgumentParser()
parser.add_argument('-p', '--project', help="Project folder",
                    action='store', metavar='<folder>', required=True)
parser.add_argument('-n', '--name', help="Name for the output manifest file",
                    action='store', metavar='<name>', required=True)
parser.add_argument('-b', '--bucket', help="Name of the S3 Bucket where the files are located",
                    action='store', metavar='<bucket>', default='webinstaller')
parser.add_argument('-s', '--url', help="Base URL of the file storage",
                    action='store', metavar='<url>',
                    default='https://s3.womolin.de')
parser.add_argument('-f', '--file', help="Partition layout csv file",
                    action='store', metavar='partitions.csv', required=True)
parser.add_argument('-o', '--outfile', help="Filename of the output manifest file",
                    action='store', metavar='<filename>', default="manifest.json")
parser.add_argument('-t', '--type', help="Manifest including all partitions (full) or just firmware/littlefs (update)",
                    action='store', metavar='<full|update>', default='update')
args = vars(parser.parse_args())

prefix = '%s/%s/%s/' % (args['url'], args['bucket'], args['project'])

data = [
    { "path": prefix + "bootloader_dio_80m.bin", "offset": 4096 },
    { "path": prefix + "partitions.bin", "offset": 32768 },
    { "path": prefix + "boot_app0.bin", "offset": 57344 },
] if args['type'] == 'full' else []

with open(args['file']) as csvfile:
    reader = csv.DictReader(csvfile, delimiter=',', skipinitialspace=True)
    for row in reader:
        row['OffsetDec'] = int(row['Offset'], base=16)
        if row['SubType'].startswith('ota_'):
            row['file'] = prefix + 'firmware.bin'
        if row['SubType'].startswith('spiffs'):
            row['file'] = prefix + 'littlefs.bin'
        
        if 'file' in row:
            data.append({ "path": row['file'], "offset": row['OffsetDec'] })

with open(os.path.dirname(__file__) + '/webinstaller-manifest-template.json') as user_file:
  manifest = json.load(user_file)
  manifest['name'] = args['name']
  manifest['version'] = currentVersion['revision']
  manifest['builds'][0]['parts'] = data
  # print(json.dumps(manifest, indent=2))

  with open(args['outfile'], "w") as out:
    print("Write manifest to %s" % args['outfile'])
    json.dump(manifest, out, indent=2)
