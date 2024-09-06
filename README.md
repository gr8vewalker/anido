# anido

An anime downloader app written in C. 
It uses [libanim](https://github.com/gr8vewalker/libanim), a web scraping anime utility library.

# Building 

This project uses Meson. So, this should be enough:
```sh
meson setup builddir
meson compile -C builddir
```

You need required dependencies (maybe their development packages too) to build.

# Dependencies

For libanim check [here](https://github.com/gr8vewalker/libanim?tab=readme-ov-file#dependencies)

Optional (in PATH):

- `ffmpeg` : Video utilities for some sources.
- `mpv`/`vlc` etc. : Playback.

# Running

Run the executable on your favorite terminal.

