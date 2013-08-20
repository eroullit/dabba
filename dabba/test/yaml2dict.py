#!/usr/bin/python
#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

import sys
import yaml

for i in range(1, len(sys.argv)):
	y = yaml.load(open(sys.argv[i]))
	print y
