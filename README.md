# anido

An anime downloader app written in C. 
It depends on `libanim`, a web scraping anime utility library.

`libanim` is still on this repo but when it gets polished enough it will be seperated.

# Building 

This project uses Meson. So, this should be enough:
```sh
meson setup builddir
meson compile -C builddir
```

You need required dependencies (maybe their development packages too) to build.

# Dependencies

Required: `libxml2`, `curl`, `openssl`

Optional (in PATH):

- `ffmpeg` : Video utilities for some sources.
- `mpv`/`vlc` etc. : Playback.

# Running

Run the executable on your favorite terminal.

