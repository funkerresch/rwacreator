# The `rwa` Pd library

The non-vanilla objects RWA Creator loads into its embedded Pd, built as a
single Pd library directory, so the player patches in `puredata/` can be opened,
heard and developed in a normal Pd before they are loaded into the Creator.

Currently only one object: `oggread~` is a fork, and patches written against it
do not work with the `pdogg` published on deken. Everything else the pooled
player patches instantiate is stock and gets installed from deken:

| Object | Where it comes from |
| --- | --- |
| `oggread~` | **here**, deken's `pdogg` is not a substitute |
| `freeverb~` | deken, library `freeverb~` |
| `vas_*`, `rwa_binauralsimple~` | the `vas` libdir built in `vas_library` |
| `earplug~` | deken, library `earplug~`, but see "Known gaps" |
| `rwa_binauralrir~` | nowhere, see "Known gaps" |

To install the deken parts: in Pd, **Help → Find externals**, search for the
library name, install. Pd puts it in your externals folder, where it works
without further path setup.

Our `freeverb~` and deken's are the same object (Olaf Matthes 1.2). The source
vendored at `pd-extra/pd/externals/freeverb~/` is byte-identical to pd-l2ork
master, and deken's package ships the same README, LICENSE, help and meta files,
so there is nothing to keep in sync and no reason to build it twice. `oggread~`
is the only object where our copy and the published one differ.

## Build and install

```bash
make
make install
```

`make install` writes the library to `~/Library/Pd/rwa`. To put it elsewhere,
for example Pd's user externals folder:

```bash
make install objectsdir=~/Documents/Pd/externals
```

Then in Pd, **Preferences → Path**, add the `rwa` folder *itself*
(`~/Documents/Pd/externals/rwa`), not its parent (same as for the
`vas` library). With the libdir on the path, plain object names work and the
existing patches load unchanged; adding only the parent would require every
object to be written namespaced as `[rwa/oggread~]`.

It also builds from the CMake project, which just calls this makefile:

```bash
cmake --build build/cmake-debug --target pd-externals
```

Ogg/Vorbis is compiled in from the vendored `libogg/` and `vorbis/` submodules,
so the binary needs no libvorbis on the machine and decodes exactly as Creator
does. Pd's headers come from the `libpd` submodule (0.52-2), the Pd embeeded into
RWA Creator and Player. To build against an installed Pd instead:

```bash
make PDINCLUDEDIR=/Applications/Pd-0.56-5.app/Contents/Resources/src
```

## `oggread~` is a fork, and the fork is the point

The object comes from Olaf Matthes' **pdogg** (0.25.1 on SourceForge, 2011; no
upstream git repo, unmaintained since). The pristine source is vendored in this
repo at `pd-extra/pd/externals/pdogg/oggread~.c`, byte-identical to pd-l2ork
master, which is the last place it is still published. Our `oggread~.c` differs
from it in three ways beyond retabbing:

- **`start` takes a float**: `[start 12.5(` starts playback 12.5 seconds in,
  required for `playheadposition` start offsets. Stock pdogg always rewinds to
  0 and ignores the argument.
- **The end-of-file bang is deferred by 1000 ms** through a second clock, rather
  than firing the moment the decoder hits EOF. The bang itself is indeed unused
  (no patch connects that outlet), but the deferral is not: it also defers the
  stop, which is what lets the buffered tail drain instead of being cut off at
  EOF, worth about 0.6 s of audio. It carries a bug, though: pending end-clock
  survives a restart.
- `oggread_stop()` moved ahead of the decoder and `x_eos` is no longer set at
  EOF, which follows from the above.

The first one is what the player patches are written against, so installing
pdogg from deken instead of this library gives patches that look like they load
and then behave differently. To see the current delta:

```bash
make check-upstream
```

## Verifying a build

On macOS externals link with a flat namespace: Pd's symbols are resolved out of
the host at load time, and the cost is that a source file left out of the build
is *not* a link error — the build succeeds and `dlopen` fails when Pd loads the
object. So check the binaries, then actually load them:

```bash
make check-symbols
```

```bash
/Applications/Pd-0.56-5.app/Contents/Resources/bin/pd -noprefs -nogui -stderr -path ~/Library/Pd/rwa -lib 'oggread~' -send 'pd quit'
```

`-noprefs`: avoid loading your configured search paths, preventing an older
loose `oggread~.pd_darwin` sitting in one of them from shadowing the one you
just built.

## Known gaps

- **`earplug~`** is used by `puredata/rwabinauralwrappermono.pd` but is not
  compiled into RWA Creator at all (there is a copy at `pd_externals/earplug~.c`
  and one in `pd-extra/`, but no `earplug_tilde_setup()` call). Installing it
  from deken makes that patch open in Pd. At some point we should investigate
  the differences of `rwa_binauralsimple~`and `earplug~`, in order to update
  projects migrated to `rwa_binauralsimple~`: There are reports of significant
  level differences!
- **`rwa_binauralrir~`** (`puredata/rwaplayermonobrir1.pd`) exists neither here
  nor in `vas_library`'s libdir. Probably a rename.
- **Linux and Windows are untested.** The makefile has the branches; nobody has
  run them.
