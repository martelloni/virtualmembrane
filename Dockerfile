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
    libboost-python-dev libboost-numpy-dev libboost-dev \
    libboost-system-dev libboost-program-options-dev

# Setup GDB debug over SSH
#RUN apt-get install -y \
#    openssh-server rsync zip

#RUN echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config
#&& \
#    echo 'PermitEmptyPasswords yes' >> /etc/ssh/sshd_config && \
#    echo 'PasswordAuthentication yes' >> /etc/ssh/sshd_config && \
#    ssh-keygen -A

#EXPOSE 22

# Passwordless root!
#RUN echo 'root:' | chpasswd

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
