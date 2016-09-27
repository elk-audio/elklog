Template for new projects.
==========================
To use, rename the project from "default" to your prefered name and start adding code

Directory layout:
---------------
### include:
Public header files should be put in this directory

### misc:
Stuff that is not directly related to the build targets, i.e. accessory scripts to generate configuration data, etc.

### src:
All source files and corresponding header files should go here. For larger projects the source files should be placed in folders separated by logical units within the project.
The template include an example `main.cpp` with logging and command line argument parsing through third-party libraries.

### test:
#### unittests:
All unit test source code goes here, the directory structure should mirror that of the src dir and the files names should be xxxx_test.cpp.

#### gtest-1.7.
The gtest framework. Automatically built with the tests.

#### tools
Examples and non-unit tests for e.g. third-party libraries.

### third_party:
Any third party dependencies that are to be compiled into the project goes here


Build instructions:
-------------------
The generate script creates a build folder and calls cmake to make a release folder and a debug folder. To rebuild, one can simply cd into build/release or build/debug and do "make".

