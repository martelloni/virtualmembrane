# GCC support can be specified at major, minor, or micro version
# (e.g. 8, 8.2 or 8.2.0).
# See https://hub.docker.com/r/library/gcc/ for all supported GCC
# tags from Docker Hub.
# See https://docs.docker.com/samples/library/gcc/ for more on how to use this image
FROM alpine:latest

# Set up C++ build environment
RUN apk update && apk add --no-cache \
    gcc g++ gdb make

# Setup GDB debug over SSH
RUN apk update && apk add --no-cache \
    openssh rsync zip

RUN echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config && \
    echo 'PermitEmptyPasswords yes' >> /etc/ssh/sshd_config && \
    echo 'PasswordAuthentication yes' >> /etc/ssh/sshd_config && \
    ssh-keygen -A

EXPOSE 22

# Passwordless root!
RUN echo 'root:' | chpasswd

# Launch SSH debugger
CMD ["/usr/sbin/sshd", "-D"]

LABEL Name=assignment3 Version=0.0.1
