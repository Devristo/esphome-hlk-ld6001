FROM esphome/esphome:latest

RUN apt update && apt-get install -y build-essential nano socat screen gdb \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*