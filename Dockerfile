FROM wiiuenv/devkitppc:20221228

COPY --from=wiiuenv/libmocha:20220919 /artifacts $DEVKITPRO
COPY --from=wiiuenv/librpxloader:20220903 /artifacts $DEVKITPRO

WORKDIR /project
