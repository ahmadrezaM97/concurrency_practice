FROM ubuntu:latest

# Update package lists and install development tools
RUN apt-get update && apt-get install -y \
    # Build tools
    gcc \
    g++ \
    make \
    cmake \
    build-essential \
    # Debugging tools
    gdb \
    strace \
    ltrace \
    valgrind \
    # Editors and utilities
    vim \
    nano \
    # Version control
    # Process monitoring
    htop \
    procps \
    psmisc \
    # File inspection tools
    file \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Set up a better vim experience
RUN echo "set number" >> /etc/vim/vimrc && \
    echo "set autoindent" >> /etc/vim/vimrc && \
    echo "set tabstop=4" >> /etc/vim/vimrc && \
    echo "set shiftwidth=4" >> /etc/vim/vimrc && \
    echo "set expandtab" >> /etc/vim/vimrc && \
    echo "syntax on" >> /etc/vim/vimrc

# Create a working directory
WORKDIR /workspace

# Copy project files
COPY . .

# Set the default shell to bash
ENTRYPOINT ["/bin/bash"]


