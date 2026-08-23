# Chow Tunes

**Note: this software is currently intended for personal use only.
I am not planning to make much effort (if any) to get this software to work
with different operating systems, compilers, platforms, or even computers that
are not my own. That said, pull requests will be accepted (following testing,
code review, etc), so if you'd like to make this software better, please jump in!**

Chow Tunes is a simple media player. It starts by scanning a directory where
the users "music library" is located. It organizes the music library into songs,
albums, and artists. From there, the user can listen to a song or album, and
can queue more songs or albums to be played next.

Chow Tunes intends to remain simple, fast, and fairly minimal.

## TODO:

I'd like to keep the feature-set pretty small, but I think there's definitely
some "low-hanging" improvements to be had.

- [ ] Make it look (a lot) nicer
- [ ] Allow the user to select their own audio device (currently we're just using the default)
- [ ] Effects
  - [ ] Equalizer
  - [ ] Safety Limiter

## Why?

I prefer to own the music that I listen to (rather than paying for a streaming service).

Since I own a fair amount of music, I need some software on my computer that I can use to
listen to all that music.

I dislike most media players, since they are often somewhat large and slow, and have
many features that I don't really care about, while missing some features that I do want.

For a long while I've used [Dopamine](https://github.com/digimezzo/dopamine), which has
treated me quite well. However, I wanted to see if I could make something a little bit
smaller and simpler that still got me the few features that I want in an audio player.

I haven't spent much time working on this, so obviously it's not very polished
and has an extremely minimal feature set, but I think it will get the job done (for now).

## License

Chow Tunes is licensed under the MIT license.

= OG

COLD:
$ ./build/ChowTunes_artefacts/Release/ChowTunes.exe
[scan] Directory enumeration + task submission: 1540 ms
[scan] Tag read histogram (per-file, parallel):  <1ms: 222  1-5ms: 5519  5-20ms: 1656  20-100ms: 145  >=100ms: 0  (slowest: 42 ms)
[scan] Tag loading (incl. thread pool wait): 2581 ms
[scan] Total scan time: 4121 ms

WARM:
$ ./build/ChowTunes_artefacts/Release/ChowTunes.exe
[scan] Directory enumeration + task submission: 77 ms
[scan] Tag read histogram (per-file, parallel):  <1ms: 7343  1-5ms: 191  5-20ms: 8  20-100ms: 0  >=100ms: 0  (slowest: 6 ms)
[scan] Tag loading (incl. thread pool wait): 251 ms
[scan] Total scan time: 329 ms

= Larger Pool

COLD:
$ ./build/ChowTunes_artefacts/Release/ChowTunes.exe
[scan] Directory enumeration + task submission: 107 ms
[scan] Tag read histogram (per-file, parallel):  <1ms: 0  1-5ms: 372  5-20ms: 5935  20-100ms: 1235  >=100ms: 0  (slowest: 80 ms)
[scan] Tag loading (incl. thread pool wait): 1657 ms
[scan] Total scan time: 1765 ms

WARM:
$ ./build/ChowTunes_artefacts/Release/ChowTunes.exe
[scan] Directory enumeration + task submission: 138 ms
[scan] Tag read histogram (per-file, parallel):  <1ms: 4977  1-5ms: 2150  5-20ms: 414  20-100ms: 1  >=100ms: 0  (slowest: 20 ms)
[scan] Tag loading (incl. thread pool wait): 118 ms
[scan] Total scan time: 256 ms
