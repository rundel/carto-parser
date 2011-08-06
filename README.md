carto-parser
--------
This is a testbed for the implementation of a native C++ parser for the carto stylesheet language. Current example implementation handles parsing of both mml and mss files with translated output into xml, dot, ast, or json depending on the file type.

Requirements
------------
- Recent revision of Mapnik2 from SVN
- Boost 1.4.7 beta 1 or newer

Building
--------
To build the example application:

	make

The included makefile has been tested on OSX 10.6.7 with Mapnik2 and Boost installed via the [Homebrew](https://github.com/mxcl/homebrew) package management system. It has not been tested on Linux or Windows, please report any issues you encounter on these or other systems. Note that the makefile currently uses clang++ as the default compiler, this can be changed to g++ by commenting and uncommenting the relevant lines.

Running
-------
The example application can be run as follows:

	./example tests/test.mml
	./example -o xml tests/test.mml
	./example -o dot tests/test.mml
	./example -o ast tests/test.mml
	./example -o json tests/test.mml
	./example tests/test.mss
	./example -o dot tests/test.mss