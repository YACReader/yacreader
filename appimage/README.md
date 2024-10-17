# AppImage

To build the AppImages of `YACReader` and `YACReaderLibrary`, podman or docker is required using an ubuntu base image of 20.04:

```
podman pull ubuntu:20.04
podman run --device=/dev/fuse --cap-add SYS_ADMIN --cap-add MKNOD --tmpfs /tmp:exec -v ./:/tmp/out --rm -ti ubuntu:20.04 /tmp/out/appimage.sh
```

The resulting AppImages will be placed in this directory.

## YACReader.c

`YACReaderLibrary` when trying to open `YACReader` will call this wrapper which will attempt to find the closest `YACReader` AppImage, then look into `PATH` and finally try to locate `YACReader` in the system if all else fails.
