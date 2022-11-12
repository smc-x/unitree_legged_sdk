#!/bin/bash
ABS=$(dirname $(realpath $0))

if [ ! -z ${https_proxy} ]; then
    PROXY_OPT="--build-arg https_proxy=${https_proxy}"
else
    PROXY_OPT=""
fi

docker build \
    ${PROXY_OPT} \
    -t smc-x/ros:arm64-melodic-unitree-sdk-$(cat ${ABS}/version) \
    .
