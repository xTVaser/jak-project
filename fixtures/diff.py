import json
import os
from pathlib import Path

def compare_configs(folder):
  # For each file in "expected", find it's pair in "new"
  # Compare the two files, yell if they don't match
  expected_files = Path("./{}/expected".format(folder)).glob('*')
  new_files = Path("./{}/new".format(folder)).glob('*')

  for expected_file in expected_files:
    new_file = None
    for file in new_files:
      if os.path.basename(expected_file) == os.path.basename(file):
        new_file = file
        break
    if new_file is None:
      print("Couldn't find a file to compare with, you did something wrong")
      exit(1)
    # Read the two files
    with open(expected_file) as f:
      expected_data = json.load(f)
    with open(new_file) as f:
      new_data = json.load(f)

    if expected_data != new_data:
      print("{} does not match {}, exiting".format(expected_file, new_file))
      exit(1)

compare_configs("ntscv1")
compare_configs("ntscv2")
compare_configs("pal")
compare_configs("jp")
