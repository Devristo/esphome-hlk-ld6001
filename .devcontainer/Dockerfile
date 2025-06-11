FROM esphome/esphome:2025.5

RUN apt update && apt-get install -y build-essential nano socat screen gdb \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*