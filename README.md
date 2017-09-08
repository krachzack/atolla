# atolla – Open Source Lighting
*atolla* is an open hardware project for makers to build exciting network programmable lighting projects in little time.

This repository provides a reference implementation of the UDP-based protocol used for communication of light information.

## Requirements
For building static and dynamic libraries, tests and examples, CMake is required.

Alternatively, if testing and examples are not required, the protocol can also be integrated into another project by adding the sources in `src` to the target project.

*atolla* is tested on Linux, Mac OS X and the ESP8266 WiFi chip. Windows, though not officially supported yet, might already work. Full Windows support will land in a future version.

## Building
1. Clone the repository with `git clone https://github.com/krachzack/atolla.git`.
2. cd into the checked out repository `cd atolla`.
3. `./configure` downloads dependencies for examples and then configures compilation of static library, dynamic library, tests and examples.
4. `cd build` after successful configuration.
5. `make` builds all of the above and creates the `build/include` directory with headers for use with the static library.

## Testing
In `build` run `make && make test`. Run `make test_pretty` for the same functionality but superior emoji support.

## Examples
Run `example_sink` to open a local sink. Next, run any of `example_sine`, `example_yellow` or `example_complementary`
with `example_sink` still running and observe the results. Source code for the examples is located in the examples
directory. The examples link against a atolla as a static library.

## Atolla Protocol
[`doc/Atolla Protocol.md`](doc/Atolla%20Protocol.md) provides a desription of the implemented
protocol.

## Changelog
| Version      | Changes                          |
|--------------|----------------------------------|
| 1.0.0        | Initial version of this reference implementation with support for Linux, Mac OS X and ESP8266 with Arduino. |