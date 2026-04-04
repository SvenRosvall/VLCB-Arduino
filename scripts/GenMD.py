#!/usr/bin/env python3

# Generate MarkDown documentation from a header file.
# This script will start generating the documentation from a line with //@Documentation
# Comment lines will be stripped off the comment marker and copied to output.
# Any other line will be copied to output with a bullet marker.

import sys
import re

documenting = False
for line in sys.stdin:
    if re.search(r'//@Documentation', line):
        documenting = True
        continue
    if re.search(r'//@End', line):
        documenting = False
        continue
    if not documenting:
        continue

    line = line.rstrip('\n\r')
    comment_match = re.match(r'^\s*//\s?(.*)', line)
    if comment_match:
        # Comment line: remove leading // and optional space
        print(comment_match.group(1))
    elif re.search(r'\S', line):
        # Non-empty line of code
        line = re.sub(r'\);$', ')', line)
        print(f"* `{line}` \\")
    else:
        # Copy blank line
        print()