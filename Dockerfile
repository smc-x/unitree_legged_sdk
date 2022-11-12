FROM arm64v8/ros:melodic-perception-bionic

RUN apt-get update \
    && apt-get install -y g++-8 \
    && rm -rf /var/lib/apt/lists/* \
    && rm /usr/bin/gcc \
    && rm /usr/bin/g++ \
    && ln -s /usr/bin/gcc-8 /usr/bin/gcc \
    && ln -s /usr/bin/g++-8 /usr/bin/g++

RUN git clone --depth 1 -b v1.4.0 https://github.com/lcm-proj/lcm \
    && cd lcm \
    && mkdir build && cd build \
    && cmake .. && make \
    && make install \
    && ldconfig \
    && rm -rf lcm

RUN apt-get update \
    && apt-get install -y tmux libzmq3-dev \
    && rm -rf /var/lib/apt/lists/*

COPY . /root/unitree_legged_sdk
RUN cd /root/unitree_legged_sdk \
    && mkdir build && cd build \
    && cmake .. \
    && make

COPY ./entrypoint.sh /entrypoint.sh
ENTRYPOINT [ "/entrypoint.sh" ]
