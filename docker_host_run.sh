#!/bin/sh

docker stop virtualmembranevscode
docker rm virtualmembranevscode
docker build -t virtualmembrane:latest -f Dockerfile .
docker run -d -p 2222:22 --security-opt seccomp:unconfined -v `pwd`:/src --name virtualmembranevscode virtualmembrane
