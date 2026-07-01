FROM rootproject/root:6.38.00-ubuntu25.10

# Install dependencies
RUN apt-get update && apt-get install -y \
    gcc-15 \
    g++-15 \
    cmake \
    nlohmann-json3-dev \
    libboost-all-dev \
    python3-full \
    python3-pip \
    python3-venv \
    libfmt-dev
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

# Make GCC 15 the default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-15 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-15 100

WORKDIR /code

# Upgrade pip and install Python packages
RUN python -m venv --system-site-packages venv
ENV PATH="/code/venv/bin:${PATH}"
RUN python -m pip install --upgrade pip && \
    pip install numpy scipy matplotlib

COPY . /code
RUN rm -rf /code/build

RUN cmake -B build && cmake --build build -j$(nproc) && cmake --install build

CMD ["/bin/bash"]
