#!/usr/bin/python
import sys
import yaml

for i in range(1, len(sys.argv)):
	y = yaml.load(open(sys.argv[i]))
	print y
