# GCC support can be specified at major, minor, or micro version
# (e.g. 8, 8.2 or 8.2.0).
# See https://hub.docker.com/r/library/gcc/ for all supported GCC
# tags from Docker Hub.
# See https://docs.docker.com/samples/library/gcc/ for more on how to use this image
FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

# Set up C++ build environment
RUN apt-get update && apt-get install -y \
    gcc g++ gdb make \
    python3-numpy python3-scipy python3-matplotlib jupyter-notebook \
    libboost-python-dev libboost-numpy-dev libboost-dev build-essential zlib1g-dev \
    libboost-system-dev libboost-program-options-dev \
    wget

# Download Catch2 for unit tests
WORKDIR /catch2
RUN wget https://raw.githubusercontent.com/catchorg/Catch2/master/single_include/catch2/catch.hpp

# LV2 plugin SDK
RUN apt-get install -y \
    python3-pip git
RUN python3 -m pip install waf
RUN git clone https://gitlab.com/lv2/lv2.git /lv2

WORKDIR /lv2
#RUN wget https://gitlab.com/lv2/lv2/-/archive/v1.18.0/lv2-v1.18.0.tar.bz2
#RUN tar jxf lv2-v1.18.0.tar.bz2
RUN git checkout v1.18.0
RUN git submodule update --init --recursive

# Launch SSH debugger
RUN apt-get update && apt-get install -y openssh-server
RUN mkdir /var/run/sshd
RUN echo 'root:notimeforsecurity' | chpasswd
#RUN sed -i 's/PermitRootLogin [^\s]+/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN echo "PermitRootLogin yes" >> /etc/ssh/sshd_config

# SSH login fix. Otherwise user is kicked off after login
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]


LABEL Name=assignment3 Version=0.0.1
