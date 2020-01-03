from ubuntu:latest

RUN apt-get update && apt-get install -y git make cmake build-essential python python3-dev python python-dev wget
RUN git clone https://github.com/openssl/openssl.git && \
  cd openssl && \
  git checkout OpenSSL_1_1_1-stable && \
  ./config && \
  make && \
  make install

RUN wget https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.gz && \
  tar -xf boost_1_71_0.tar.gz && \
  cd boost_1_71_0 && \
  ./bootstrap.sh && \
  ./b2 link=static && \
  ./b2 link=static install

RUN git clone https://github.com/open62541/open62541.git && \
  cd open62541 && \
  git checkout 1.0 && \
  git submodule update --init --recursive && \
  mkdir build && cd build && \
  cmake -DBUILD_SHARED_LIBS=FALSE -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUA_NAMESPACE_ZERO=FULL -DUA_ENABLE_SUBSCRIPTIONS_EVENTS=ON .. && \
  make && \
  make install

RUN git clone https://github.com/mtconnect/open62541_ua_server.git && \
  cd open62541_ua_server && \
  mkdir build && cd build && \
  cmake .. && \
  make 
