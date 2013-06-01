What is dabba?
==============

[![Build Status](https://secure.travis-ci.org/eroullit/dabba.png?branch=master)](http://travis-ci.org/eroullit/dabba)

dabba is a set of network tools written for Linux.

This is inspired on the Dabba Walla system present in Mumbai where
meals are regrouped and dispatched throughout the city with a high
efficiency rate, every day of the year.

The project has 4 main components:
* `libdabba` - low-level zero-copy network library
* `libdabba-rpc` - protobuf-based rpc library for dabbad and dabba
* `dabbad` - multi-threaded task manager and RPC query processor
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
* `build-essential`
* `cmake`
* `pthread`
* `libnl-3-dev`
* `libnl-genl-3-dev`
* `libnl-route-3-dev`
* `protobuf-c-compiler`
* `libprotobuf-c0-dev`
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

Testing
=======

Automatic tests are available. There are two kinds:
* Read-only (which do not interfere with network interface settings)
* Read-write (which modify network interface settings)

WARNING: Read-write tests might degrade or disable the
interface connectivity while tests are running. Avoid to run
then on production interfaces.

To run read-only automatic tests, make sure proper capabilities are granted and
run in the build directory:
```sh
$ ctest --output-on-failure
```

To run read-only automatic tests, make sure proper capabilities are granted,
set the `TEST_DEV` variable with the interface name to modify
and run in the build directory:
```sh
$ TEST_DEV=eth0 ctest --output-on-failure
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
$ dabba capture get
---
  captures:
    - id: 3076324160
      rc: 0 # Success
      packet mmap size: 32768
      frame number: 16
      pcap: /tmp/example.pcap
      interface: eth0
```

Stopping the capture:
```sh
$ dabba capture stop --id 3076324160
```

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

[CODING]: https://github.com/eroullit/dabba/blob/master/CODING
