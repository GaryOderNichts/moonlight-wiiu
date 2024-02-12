# Moonlight Wii U

Moonlight Wii U is a port of [Moonlight Embedded](https://github.com/moonlight-stream/moonlight-embedded), which is an open source client for [Sunshine](https://github.com/LizardByte/Sunshine) and NVIDIA GameStream.

Moonlight Wii U allows you to stream your full collection of games from your powerful Windows desktop to your Wii U.

## Quick Start

* A Wii U LAN Adapter is recommended
* Grab the latest version from the [releases page](https://github.com/GaryOderNichts/moonlight-wiiu/releases) and extract it to the root of your SD Card
* Enter the IP of your GFE server in the `moonlight.conf` file located at `sd:/wiiu/apps/moonlight`
* Ensure your GFE server and Wii U are on the same network
* Turn on Shield Streaming in the GFE settings
* Pair Moonlight Wii U with the GFE server
* Accept the pairing confirmation on your PC
* Connect to the GFE Server with Moonlight Wii U
* Play games!

## Configuration

You can configure all of the documented settings in the `moonlight.conf` file located at `sd:/wiiu/apps/moonlight`.

## Supported controllers

* Gamepad (can be disabled with the `disable_gamepad` option)
* Up to 4 Wii U Pro Controllers and Wii Classic Controllers (Pro)  
  Gamepad needs to be disabled to use the 4th controller

## See also

[Moonlight-common-c](https://github.com/moonlight-stream/moonlight-common-c) is the shared codebase between different Moonlight implementations

## Contribute

1. Fork us
2. Write code
3. Send Pull Requests

## Building from source

You can simply build this using the provided Dockerfile.  
Use `docker build -t moonlightbuilder .` to build the container.  
Then use `docker run -it --rm -v ${PWD}:/project moonlightbuilder make` to build moonlight.  
