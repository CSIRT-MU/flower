# FLOWer IPFIX Exporter

Flower is a IPFIX flow exporter designed to export variable IPFIX records.

## Usage

Currently flower has two input plug-ins, file input and interface input.
File input is the default choice and can be run using just command:

`./flower <FILE>`

To process data captured on interface the input plug-in must be changed, either
by using `--input_plugin` or `-I` flag:

`./flower -I InterfaceInput <INTERFACE>`

Where interface can be for example `wlp3s0`. Flower also has other options, such as:

- `--export_interval` that takes seconds as argument

Also, Flower can print all input plug-ins using flag `--list_plugins`. If you
prefer configuration from a file Flower reads its configuration file from
standard locations:

TODO: Add standard locations

## Installation

TODO: Add installation into CMAKE
