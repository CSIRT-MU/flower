# FLOWer IPFIX Exporter

Flower is a IPFIX flow exporter designed to export variable IPFIX records.

## Usage

Currently flower has two input plug-ins, file input and interface input.
File input is the default choice and can be run using a command:

`./flower process <FILE>`

To process data captured on an interface the input plug-in must be changed,
either by using `--input_plugin` or `-I` flag:

`./flower process <INTERFACE> -I InterfaceInput`

Where interface can be for example `wlp3s0`. Flower also has other options, such as:

- `--idle_timeout` that takes seconds as argument
- `--active_timeout` that takes seconds as argument

Also, Flower can print all input plug-ins using command `plugins`. If you
prefer configuration from a file Flower reads its configuration file from
standard locations. By default flower will try to open configuration file at
`~/.flower.conf`. The configuration file can look as follows:

```
# Flower config file
# Format:
# Key=Value

ip_address=127.0.0.1
port=20000
```

## Installation

TODO: Add installation into CMAKE
