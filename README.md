[![CI-Release](https://github.com/wiiu-env/AromaUpdater/actions/workflows/ci.yml/badge.svg)](https://github.com/wiiu-env/AromaUpdater/actions/workflows/ci.yml)

# Aroma Updater

The Aroma Updater can be used to update (or install) "packages" from https://aroma.foryour.cafe/ directly on your console. 

## Usage

1. Copy the file `AromaUpdater.wuhb` into `sd:/wiiu/apps/AromaUpdater.wuhb`.
2. Run the Updater via Aroma (e.g. from the Wii U Menu).

## Buildflags

### Logging
Building via `make` only logs errors (via OSReport). To enable logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule) set `DEBUG` to `1` or `VERBOSE`.

`make` Logs errors only (via OSReport).  
`make DEBUG=1` Enables information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).  
`make DEBUG=VERBOSE` Enables verbose information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).

If the [LoggingModule](https://github.com/wiiu-env/LoggingModule) is not present, it'll fallback to UDP (Port 4405) and [CafeOS](https://github.com/wiiu-env/USBSerialLoggingModule) logging.

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t aromaupdater-builder

# make 
docker run -it --rm -v ${PWD}:/project aromaupdater-builder make

# make clean
docker run -it --rm -v ${PWD}:/project aromaupdater-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./source --exclude ./source/utils/json.hpp -i`

## Credits

- Thanks Ingunar for the logo