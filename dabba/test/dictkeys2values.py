#!/usr/bin/python
#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

import sys

y=eval(sys.stdin.read())

for i in range(1, len(sys.argv)):
	if sys.argv[i].isdigit():
		sys.argv[i] = int(sys.argv[i])
	y = y[sys.argv[i]]

print y
