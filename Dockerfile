FROM ghcr.io/wiiu-env/devkitppc:20221228

COPY --from=ghcr.io/wiiu-env/libmocha:20220919 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/librpxloader:20220903 /artifacts $DEVKITPRO

WORKDIR /project
