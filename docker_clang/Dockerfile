FROM ubuntu:18.04

# install some base components
RUN apt-get update && apt-get install -y \
    tree htop nano vim curl wget \
    zsh fish \
    python3 python3-pip \
    git cmake ninja-build m4

# install zsh & oh-my-zsh (so that we have nice colors)
RUN apt-get install -y zsh
RUN wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true

# Prompt with a [DOCKER] prefix
RUN echo "\
export PROMPT='[DOCKER]\${ret_status} %{\$fg[cyan]%}%~%{\$reset_color%} > '\
" >> /root/.zshrc

# Correct locale
ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8

# install clang
RUN mkdir /clang && \
    cd /clang && \
    wget http://releases.llvm.org/8.0.0/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz && \
    tar xvf clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz && \
    rm clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
ENV CLANG_PATH /clang/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-18.04
ENV PATH $CLANG_PATH/bin:$PATH
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CLANG_PATH/lib
ENV CC clang
ENV CXX clang++

# install conan
RUN pip3 install conan
RUN conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
RUN conan remote add ericniebler https://api.bintray.com/conan/range-v3/range-v3
# configure conan default compiler: copy profile
ADD conan_default.txt /root/.conan/profiles/default
# pre-install some conan packages
RUN conan install fmt/5.3.0@bincrafters/stable --build=missing
RUN conan install spdlog/1.3.1@bincrafters/stable --build=missing
RUN conan install boost/1.69.0@conan/stable --build=missing
RUN conan install gmp/6.1.2@bincrafters/stable --build=missing
RUN conan install range-v3/0.4.0@ericniebler/stable --build=missing
RUN conan install jsonformoderncpp/3.6.1@vthiery/stable

RUN apt-get install libgmp-dev -y

# end : set workdir
WORKDIR /sources_docker
