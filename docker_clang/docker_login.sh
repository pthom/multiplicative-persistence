DIR="$(dirname "$(greadlink -f "$0")")" # readlink on linux (brew install coreutils on mac)
docker run --rm -it -v $DIR/..:/sources_docker clang_devel /bin/zsh
