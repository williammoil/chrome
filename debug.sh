#!/usr/bin/env bash
set -e

version=$(<version)

DEBUG_DIR="$(pwd)"/debug
CHROME_DATA_DIR="$DEBUG_DIR"/profile
CHROME_DIR=/opt/google/chrome/

mkdir -p $DEBUG_DIR
mkdir -p $CHROME_DATA_DIR
echo "Chrome user profile files are in $CHROME_DATA_DIR"

# Environment variables
export NACL_PLUGIN_DEBUG=1
export NACL_SRPC_DEBUG=255
export NACLVERBOSITY=255

export NACL_EXE_STDERR="$DEBUG_DIR"/stderr.log
rm -f "$NACL_EXE_STDERR"

export NACL_EXE_STDOUT="$DEBUG_DIR"/stdout.log
rm -f "$NACL_EXE_STDOUT"

NCPUS="1"
if [ -n `which nproc` ]; then
    NCPUS=`nproc`
fi

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH":"$CHROME_DIR"

echo "$(tput setaf 2)Launching Chrome$(tput sgr 0)"

# launch chrome directly
"$CHROME_DIR"/chrome --user-data-dir="$CHROME_DATA_DIR" --test-type --no-first-run --no-sandbox \
                     --load-extension="$(pwd)"/build/"$version" \
                     --enable-logging=stderr --disable-default-apps \
                     --enable-nacl --vmodule=ppb*=4
