THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

pushd $THIS_DIR

BUILD_TYPE="Debug"

#!/bin/bash
if [ "$1" == "--release" ]; then
    BUILD_TYPE="Release"
fi

make V=1 TOOLCHAIN=clang-newlib CONFIG="$BUILD_TYPE" VALID_ARCHES='arm x86_64'

if [ $? -eq 0 ]; then
  echo "$(tput setaf 2)Build successful$(tput sgr 0)"
else
  echo "$(tput setaf 1)Build failed$(tput sgr 0)"
fi

popd