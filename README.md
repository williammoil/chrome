# App

> The CTox Source Code for Google Chrome.

[![license: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue.svg?style=flat-square)](https://www.gnu.org/licenses/gpl-3.0.html)
[![js-standard-style](https://img.shields.io/badge/code%20style-standard-blue.svg?style=flat-square)](https://github.com/feross/standard)

This project aims to run [Tox](https://tox.chat/) in Google Chrome's [Native Client](https://developer.chrome.com/native-client) sandbox environment.

The app is not yet complete, so you will probably not find much use for it
unless you're a developer wishing to tinker with it.

## Getting Started

### Prerequistes

The minimum requirements to build the library are:

- libc `>=2.1`
- python `>=2.7`
- node `>=4.2.2`
- GraphicsMagick `>=1.3.1`
- [Native Client SDK](https://developer.chrome.com/native-client/sdk/download)
- Pepper API `>=49` (part of Native Client SDK)

### Preparing your system

1. Build the [toxcore-native-client](https://github.com/ctox/toxcore-native-client) webports package.

2. Install the node.js dependencies

    ```sh
    $ npm install
    ```

3. Create a private key called `chrome.pem` in the root directory of the project

    ```sh
    $ openssl genrsa 2048 | openssl pkcs8 -topk8 -nocrypt -out chrome.pem
    ```

## Building

Before you can build the app with the Native Client SDK toolchain, you must
set the `NACL_SDK_ROOT` environment variable to top directory of a version of the
Pepper API (the directory containing `toolchain/`). This path should be absolute

```
export NACL_SDK_ROOT=<INSTALLATION_DIR>/nacl_sdk/pepper_<VERSION>
```

There are 3 possible architectures that the nacl module can be compiled for:
~~`i686`~~, `x86_64` and `arm`.

The build script will compile architecture-specific `.nexe` files for ~~`i686`~~, `x86_64` and `arm`
files and generate a Native Client manifest (.nmf) file into the project's root `clang-newlib/`
directory automatically

```sh
$ sh build.sh
```

By default all builds are in debug configuration. If you want to build release
packages add the `--release` flag

```sh
$ sh build.sh --release
```

Now run the gulpfile to build the source code and to package a `.crx` file into the project's
root directory, which is the actual app that can be installed

```sh
$ npm run build
```

## Installing

You can either install the packaged `.crx` file from the project's root directory or
load an unpacked extension from `build/<VERSION>` (recommended). Please notice,
that you'll need to launch Chrome with debug flags in order to use to use POSIX-like
sockets (`nacl_io`) locally.

## Debugging

Use the debug script, to launch a new Chrome instance with debug flags and a blank profile.

A new user data directory will be created in the project's root `debug/profile/`
directory. If you launch Chrome this way, be sure it doesn’t attach to an existing instance

```sh
$ npm run debug
```

By default stdout and stderr will appear in Chrome’s stdout and stderr stream in
the terminal, but to simplify things, we redirect them to log files. You can find
them in the project's root `debug/` directory.


