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
* `setcap` (within `libcap2-bin` package)

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

The MIT License (MIT)

Copyright (c) 2013 Emmanuel Roullit

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Contact
=======

For questions:
    Send me a message to @eroullit on GitHub

For suggestions or bug reports:
    Feel free to open an issue on the [ticket system](https://github.com/eroullit/dabba/issues).

[CODING]: https://github.com/eroullit/dabba/blob/master/CODING
