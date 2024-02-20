# Moonlight Wii U

Moonlight Wii U is a port of [Moonlight Embedded](https://github.com/moonlight-stream/moonlight-embedded), which is an open source client for [Sunshine](https://github.com/LizardByte/Sunshine) and NVIDIA GameStream. Moonlight allows you to stream your full collection of games and applications from your PC to other devices to play them remotely.

## Quick Start

> :information_source: A Wii U LAN Adapter is recommended!

* Grab the latest version from the [releases page](https://github.com/GaryOderNichts/moonlight-wiiu/releases) and extract it to the root of your SD Card.
* Enter the IP of your Sunshine/GFE server in the `moonlight.conf` file located at `sd:/wiiu/apps/moonlight`.
* Ensure your Sunshine/GFE server and Wii U are on the same network.
* If using GFE, turn on Shield Streaming in the GFE settings.
* Pair Moonlight Wii U with the server.
* Accept the pairing confirmation on your PC.
* Connect to the server with Moonlight Wii U.
* Play games!

## Configuration

You can configure all of the documented settings in the `moonlight.conf` file located at `sd:/wiiu/apps/moonlight`.  
Note that a lot of option are commented out by default, to edit them you need to remove the `#` in front of them.

## Supported controllers

* Gamepad (can be disabled with the `disable_gamepad` option).
* Up to 4 Wii U Pro Controllers and Wii Classic Controllers (Pro).  
  The Gamepad needs to be disabled to use the 4th controller.

## Troubleshooting
### Input doesn't work when using Sunshine
Verify that you've installed [Nefarius Virtual Gamepad](https://github.com/nefarius/ViGEmBus/releases/latest) and restarted your PC after the installation.

### The stream disconnects frequently/immediately
Depending on your network connection you need to adjust the configuration to find a stable bitrate and resolution.
Try something like this to get started:
```
width = 854
height = 480
fps = 30
```
```
bitrate = 1500
```
Then slowly increase the bitrate until the stream is no longer stable.

### Can't find app Steam
Moonlight Wii U tries to start the app "Steam" by default, but sunshine does not have a default Application called "Steam".  
You can either rename the app in the `moonlight.conf` to
```
app = Steam Big Picture
```
which is a default option in sunshine or add a new application called "Steam" in the sunshine configuration.

## See also

[Moonlight-common-c](https://github.com/moonlight-stream/moonlight-common-c) is the shared codebase between different Moonlight implementations

## Contribute

1. Fork us
2. Write code
3. Send Pull Requests

## Building from source
Install the required dependencies: `(dkp-)pacman -S --needed wiiu-dev wiiu-sdl2 wiiu-curl wiiu-mbedtls ppc-freetype ppc-libopus ppc-libexpat`.  
Run `make` to build moonlight.

### Using docker
You can also build moonlight-wiiu using the provided Dockerfile.  
Use `docker build -t moonlightbuilder .` to build the container.  
Then use `docker run -it --rm -v ${PWD}:/project moonlightbuilder make` to build moonlight.  
