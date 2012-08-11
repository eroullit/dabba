What is dabba?
==============

dabba is a set of network tools written for Linux.

The project has 3 main components:
    * `libdabba` - low-level zero-copy network library
    * `dabbad` - multi-threaded task manager and IPC query processor
    * `dabba` - CLI 'à la git' to communicate with dabbad and submit tasks

Currently supported feature are listed in the [CODING] file.

Get the source!
===============

```
git clone git://github.com/eroullit/dabba.git
```

Contributions are welcome, [CODING] regroups all information about it.

Prerequisites
=============

dabba needs these to compile and run properly:
	* Linux kernel >= 2.6.31
	* `cmake`
	* `pthread`
	* `setcap` (within `libcap` package)

Recommended optional packages:
	* `python-yaml`
	* `doxygen`

Installation
============

To build out-of-tree:
```sh
$ mkdir build && cd build && cmake .. && make
```

To install it after being built:
```sh
$ sudo make install/strip
```

Capabilities
============

To configure dabba to run with a regular user account:
```sh
$ sudo make setcap
```

Getting started!
================

Start dabbad:
```sh
$ dabbad --daemonize
```

As long as dabbad is running, captures tasks can be submitted:
```sh
$ dabba capture start --interface eth0 --pcap /tmp/example.pcap
```

Running captures can be listed in YAML format:
```sh
$ dabba capture list
---
  captures:
    - id: 3076324160
      packet mmap size: 32768
      frame number: 16
      pcap: /tmp/example.pcap
      interface: eth0
```

Stopping the capture:
```sh
	$ dabba capture stop --id 3076324160
```

More info
=========

Further information can be found on the project's [wiki](https://github.com/eroullit/dabba/wiki)

Copyright
=========

Copyright (C) 2011-2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110, USA

Contact
=======

For questions:
    Send me a message to @eroullit on GitHub

For suggestions or bug reports:
    Feel free to open an issue on the [ticket system](https://github.com/eroullit/dabba/issues).
