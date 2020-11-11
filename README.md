# FLOWer IPFIX Exporter

Flower is a IPFIX flow exporter designed to export variable IPFIX records.

## Usage

Currently flower has two input plug-ins, file input and interface input.
File input is the default choice and can be run using a command:

`flower process <FILE>`

To process data captured on an interface the input plug-in must be changed,
either by using `--input_plugin` or `-I` flag:

`flower process <INTERFACE> -I InterfaceInput`

Where interface can be for example `wlp3s0`. Flower also has other options, such as:

- `--idle_timeout` that takes seconds as argument
- `--active_timeout` that takes seconds as argument

Also, Flower can print all input plug-ins using command `plugins`. If you
prefer configuration from a file Flower reads its configuration file from
standard locations. By default flower will try to open configuration file at
`~/.flower.conf`. The configuration file can look as follows:

```
# Flower config file

[ip]
src = true
dst = true

[ipv6]
src = true
dst = true

[tcp]
src = true
dst = true

[udp]
src = true
dst = true

[vlan]
id = true

[vxlan]
vni = true

[ethernet]
src = true
dst = true

[gre]
[mpls]
```

### Example usage

To print available plugins:

`flower plugins`

To print system plugins and plugins in current directory:

`flower --plugins_dir . plugins`

To process PCAP file with debug logging:

`flower --debug process file.pcap`

To capture and process packets from live interface with specific active and
idle timeout:

`flower process wlp1s0 -I InterfaceInput -i 15 -a 120`

## Installation

Flower uses `libtins` so it needs to be installed in the system. Other
dependencies are downloaded using CMake's `FetchContent`. Building and
installing Flower in project directory:

```
# In project directory
mkdir build && cd build
cmake ..
make
sudo make install
```

## Creating custom input plugin

Flower's input functionality can be extended by adding input plugins. To build
input plugin one needs implement input plugin interface. All structures and
types are defined in `input.h` and `plugin.h`.  It is possible to add Flower as
a dependency into `CMakeLists.txt`. An example `CMakeLists.txt` of a plugin can
look as follow:

```
cmake_minimum_required(VERSION 3.12)

project(dummy_plugin VERSION 0.1.0)

find_package(flower REQUIRED)

add_library(dummy_provider MODULE dummy.c)
target_include_directories(dummy_provider PRIVATE flower)
```

An example input plugin implementation can be seen in `plugins/file_provider.c`.
