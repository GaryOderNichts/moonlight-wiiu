# Moonlight Wii U

Moonlight Wii U is a port of Moonlight Embedded, which is an open source implementation of NVIDIA's GameStream, as used by the NVIDIA Shield.

Moonlight Wii U allows you to stream your full collection of games from your powerful Windows desktop to your Wii U.

## Requirements

* [GFE compatible](http://shield.nvidia.com/play-pc-games/) computer with GTX 600/700/900/1000 series GPU (for the PC you're streaming from)
* Geforce Experience 2.1.1 or higher
* A Wii U LAN Adapter is recommended

If your PC isn't supported or you're having performance related issues, try using [sunshine](https://github.com/loki-47-6F-64/sunshine) instead.

## Quick Start

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
