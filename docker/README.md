These are the official Dockerfile for YACReaderLibraryServer.

By default the images will be created using the `develop` branch from YACReader's Github repository. You can pass  the `YACR_VESRION` argument with a tag to build one of the relase versions, e.g. `ARG YACR_VERSION=9.15.0`

This is an example about how to run YACReaderLibraryServer:

```
docker run -d --name YACReaderLibraryServer -e PUID=99 -e PGID=100 -e TZ=Europe/Madrid -p 9999:8080 -v 'C:\Users\my_user_name\Desktop\:/config' -v 'C:\Users\my_user_name\Desktop\MyLibrary:/comics' --restart unless-stopped yacreaderlibraryserver:latest
```

`-p` sets the port that needs to be used in the iOS and Android apps, in the example above the port would be 9999. This port connects with the port 8080 in the container, and that would be the port that the server will be seeing.
`-v 'path:/comics'` sets the path to the folder containing your library in your real machine. This path is connected to the /comics volumen, and this is the path that the server will seen in the container.

`root.tar.gz` contains some default configuration + `s6-overlay` files that take care of keeping `YACReaderLibraryServer` running (and starting it automatically when the docker image starts), the files inside need to be carefully edited in linux to avoid having trobules at runtime, since I normally develop on Windows the `tar.gz` keeps the integrity of the files.

The images are pushed to `yacreader/yacreaderlibraryserver` in Docker Hub. The tags `:develop`, `:latest` and the version number are available. `:develop` will contain an image with the latest dev build, while `:latest` will point to the latest stable release.
