FROM ubuntu

RUN apt-get update 
RUN apt-get upgrade -y
RUN apt-get install -y nasm

VOLUME /root/env
WORKDIR /root/env
