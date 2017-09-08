# Atolla – Open Source Lighting
Atolla is an open hardware project for makers to build exciting network programmable lighting projects in little time.

This repository provides a reference implementation of the UDP-based protocol used for communication of light information.

## Building
1. Clone the repository with `git clone https://github.com/krachzack/atolla.git`.
2. cd into the checked out repository `cd atolla`.
3. `./configure` downloads dependencies for examples and then configures compilation of static library, dynamic library, tests and examples, the working directory changes to `build` after successful configuration.
4. `make` builds all of the above and creates the `build/include` directory with headers for use with the static library.

## Testing
In the top level directory of the repository, run `./test`.

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
| 1.0          | Initial version of this reference implementation. |