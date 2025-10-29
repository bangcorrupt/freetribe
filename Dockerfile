FROM debian:bookworm

RUN apt-get update && \
    apt-get clean && \ 
    apt-get install -y --no-install-recommends \
    build-essential \
    gcc-arm-none-eabi \
    gdb-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    picolibc-arm-none-eabi \
    openocd \
    python3 \
    python3-pip \
    git \
    wget \
    ftp \
    xxd \
    neovim \
    sudo

RUN wget --progress=dot:giga --no-check-certificate --content-disposition -c \
    https://sourceforge.net/projects/adi-toolchain/files/2014R1/2014R1-RC2/x86_64/blackfin-toolchain-elf-gcc-4.3-2014R1-RC2.x86_64.tar.bz2

RUN tar -xjvf blackfin-toolchain-elf-gcc-4.3-2014R1-RC2.x86_64.tar.bz2?viasf=1 -C / && \
    rm /blackfin-toolchain-elf-gcc-4.3-2014R1-RC2.x86_64.tar.bz2?viasf=1

# Add openocd-bfin

ENV PATH "/opt/uClinux/bfin-elf/bin/:$PATH"
ENV LIB_GCC="/usr/lib/gcc/arm-none-eabi/12.2.1/"
ENV LIB_C="/usr/lib/arm-none-eabi/newlib/"
ENV BFIN_TOOLCHAIN="/opt/uClinux/bfin-elf/bin/"

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
RUN useradd -G sudo -ms /bin/bash user && \
    echo "user:password" | chpasswd && \
    echo "ALL ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers && \
    echo "alias python=python3" >> /home/user/.bashrc && \
    echo "alias ll='ls -lah'" >> /home/user/.bashrc && \
    echo "alias vim=nvim" >> /home/user/.bashrc

USER user
WORKDIR /freetribe

ENTRYPOINT ["/bin/bash"]
