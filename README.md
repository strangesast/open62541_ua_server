MTConnect OPC UA Gateway server, **opcua-MTServer**, implements MTConnect OPC UA Companion Specification. It connects to MTConnect agents for the information of MT devices and monitors their real-time data.

Building
-------

**opcua-MTServer** is written in C++ using **open62541**, **boost** and **openssl** libraries. It uses **CMake** as the build system. First download and install them:

- [open62541](https://open62541.org/)
- [CMake](https://cmake.org)
- [boost](https://www.boost.org)
- [openssl](https://www.openssl.org) - This is to support https secure protocol. For Windows, after the build, prepend the location of libcrypto.dll and libssl.dll to the PATH system variable.

Then run these commands:

- **cmake Makefiles.list**
- **cmake --build . --target all**

If build successful, **opcua-MTServer** should be generated in the current directory.

Usage:

- opcua-MTServer **[MTConnect Agent URL address] [poll cycle in seconds - default is 60]**
  
Example:  opcua-MTServer https://smstestbed.nist.gov/vds
or

- opcua-MTServer **[configuration file]**

Example:  opcua-MTServer opcua.cfg

opcua.cfg allows connections to multiple MTConnect Agents. Each line contains <agent's url> and its poll frequency in seconds.

