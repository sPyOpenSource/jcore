FROM ubuntu

RUN apt-get update 
RUN apt-get upgrade -y
RUN apt-get install -y nasm
RUN apt-get install -y make
RUN apt-get install -y gcc-x86-64-linux-gnu
RUN apt-get install -y c++-x86-64-linux-gnu
RUN apt-get install -y llvm
RUN apt-get install -y clang

VOLUME /root/env
WORKDIR /root/env
