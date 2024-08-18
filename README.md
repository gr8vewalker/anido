# Anime Downloader

An anime downloader app written in C. Made exclusively for TurkAnime and it's player Alucard. (Multi-site and other sources support coming soon!)

No official Windows support yet!

# Building 

Use `./scripts/build.sh` or `./scripts/build.sh release` for release builds.

# Usage

### Dependencies

Required: 

- `libxml2` : Web scraping.
- `curl` : Getting websites and downloading.
- `openssl` : Base64 decode and AES decryption.

Optional: (these libraries should be in PATH)

- `ffmpeg` : Some sources needs concatenating videos etc.
- `mpv`/`vlc` : Playing the video in stream mode. (Default: mpv)

### Running

```anido [-s] [-p player]```

Run the executable on your favorite terminal.

If you can't see the symbols, use a font that supports them.
