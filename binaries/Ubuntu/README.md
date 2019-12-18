## Steps to build opcua-MTServer in Ubuntu

- Install prerequisites git, make and cmake if not already
	- apt install git
	- apt install make
	- apt install cmake
	
- Download and build openssl
	- git clone https://github.com/openssl/openssl.git
	- cd openssl
	- git checkout OpenSSL_1_1_1-stable
	- ./config
	- make
	- sudo make install

- Download and build boost
	- wget https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.gz
	- tar zxvf boost*.gz
	- cd boost_1_*
	- ./bootstrap.sh
	- ./b2 link=static
	- sudo ./b2 link=static nstall

- Download and build open62541
	- git clone https://github.com/open62541/open62541.git
	- cd open62541
	- git checkout 1.0
	- git submodule update --init --recursive
	- mkdir build && cd build
	- cmake -DBUILD_SHARED_LIBS=FALSE -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUA_NAMESPACE_ZERO=FULL -DUA_ENABLE_SUBSCRIPTIONS_EVENTS=ON ..
	- make
	- sudo make install

-  Download and build open62541_ua_server
	- git clone https://github.com/mtconnect/open62541_ua_server.git
	- cd open62541_ua_server
	- mkdir build && cd build
	- cmake ..
	- make
	- check if opc-MTServer is built in the current directory

