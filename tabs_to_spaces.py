#!/usr/bin/env python3

import sys
import os
import random

def tabs_to_spaces(file_contents, tab_width):
  lines = []
  for line in file_contents.split("\n"):
    while True:
      tab_index = line.find("\t")
      if tab_index == -1: break
      space_count = tab_width - tab_index % tab_width
      line = line[:tab_index] + (" " * space_count) + line[tab_index+1:]
    lines.append(line)
  return "\n".join(lines)

def main(paths, tab_width):
  for path in paths:
    if path == "-":
      old_contents = sys.stdin.read()
    else:
      with open(path) as f:
        old_contents = f.read()
    new_contents = tabs_to_spaces(old_contents, tab_width)
    if path == "-":
      # always output stdout
      sys.stdout.write(new_contents)
      continue
    if old_contents == new_contents:
      continue
    # there's a difference
    tmp_path = path + "." + hex(random.randint(0x1000000000000000, 0xffffffffffffffff)) + ".tmp"
    with open(tmp_path, "w") as f:
      f.write(new_contents)
    os.rename(tmp_path, path)

def cli():
  import argparse
  parser = argparse.ArgumentParser()
  parser.add_argument("files", nargs="*", default=["-"], help="defaults to -, meaning stdin/stdout")
  parser.add_argument("-t", "--tab-width", type=int, default=8, help="defaults to 8")
  args = parser.parse_args()
  if args.tab_width <= 0: parser.error("--tab-width must be positive")
  main(args.files, args.tab_width)

if __name__ == "__main__":
  cli()
