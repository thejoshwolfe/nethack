#!/usr/bin/env python

import re

def main(input_path, output_path):
  with open(input_path) as f:
    input_contents = f.read()

  definitions = []
  enum_declarations = re.findall(r"typedef enum \{(.*?)\} (.*?);", input_contents, re.DOTALL)
  for (contents, enum_name) in enum_declarations:
    mapping = []
    names_and_values = re.findall(r"^    {}_(\w+)(?: = (.+?))?,".format(enum_name), contents, re.MULTILINE)
    last_value = -1
    for (name, value) in names_and_values:
      if not value:
        value = str(last_value + 1)
      mapping.append((name, value))
      last_value = int(value, 0)
    definitions.append((enum_name, mapping))

  # ... maybe this is a little TOO clever.
  output_contents = "\n\n".join("exports.%s = {\n%s\n};" % (enum_name, "\n".join("  {}: {},".format(name, value) for (name, value) in mapping)) for (enum_name, mapping) in definitions) + "\n"
  with open(output_path, "w") as f:
    f.write(output_contents)

def cli():
  import argparse
  parser = argparse.ArgumentParser()
  parser.add_argument("input", metavar="input.h")
  parser.add_argument("-o", "--output", metavar="output.js", required=True)
  args = parser.parse_args()
  main(args.input, args.output)

if __name__ == "__main__":
  cli()
