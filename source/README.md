# macse-warpspeed

PiStorm Macintosh SE experiment based on the verified `pistorm_macse_physical_write_filter` build.

This build boots a 4 MB Macintosh SE while reducing physical-RAM write traffic. It keeps the complete Pi-side CPU RAM mapping and writes only explicitly configured RAM ranges through to physical Macintosh RAM.

## Verified result

The native Raspberry Pi build booted the Macintosh SE and was reported to run approximately 3× faster than the previous baseline according to CPU benchmarks.

Verified Pi artifact:

```text
pistorm_macse_physical_write_filter
SHA-256: 8ab39414a501824940dea3c3be6ba66d1a26b7aa91abaf6df2591aa5f1f09574
```

The binary is intentionally not committed to this repository. Build it natively on the target Pi.

## What changed

This experiment adds a configurable physical-write policy without splitting the Macintosh memory map and without creating a second data cache.

### CPU RAM remains complete

The configuration retains one full 4 MB Pi-side `wtcram` mapping:

```text
map type=wtcram address=0x000000 size=4M id=sysram
```

Vectors, stack, heap, filesystem buffers, driver workspace, and ordinary CPU RAM remain available in the Pi-side backing store.

### Configurable physical write-through

The new configuration directive is:

```text
physwrite address=ADDRESS size=SIZE
```

When one or more `physwrite` directives are present:

- writes in configured ranges continue to the physical Macintosh RAM bus;
- writes to ordinary RAM outside those ranges are captured into the existing Pi-side RAM backing store;
- device and ROM-space accesses at or above `0x400000` remain on the normal physical path;
- byte, word, unaligned word, and longword writes are handled correctly;
- no `/DTACK` polling, refresh loop, queue, dirty scan, or secondary cache is used.

The implementation registers the complement of the configured ranges as local Musashi write ranges. This is necessary because `wtcram` normally lets CPU writes fall through to the physical bus; simply skipping `ps_write()` would leave the Pi-side cache stale.

## Current 4 MB configuration

The supplied configuration is:

```text
cpu 68000
platform macse
map type=rom address=0x400000 size=1M file=/home/pi/pistorm_latest_09172026/macse.rom id=sysrom
map type=wtcram address=0x000000 size=4M id=sysram

physwrite address=0x100000 size=1M
physwrite address=0x3F2700 size=0x5580
physwrite address=0x3FA100 size=740
physwrite address=0x3FA700 size=0x5580
physwrite address=0x3FFD00 size=740

loopcycles 300
```

The first range is an empirical diagnostic range required for reliable floppy boot in the current hardware/software path. It is not claimed to be a documented fixed floppy buffer and should not be treated as a final architecture.

The remaining ranges are the 4 MB Macintosh SE BBU-selected areas:

```text
Alternate video:       0x3F2700–0x3F7C7F
Alternate sound/PWM:   0x3FA100–0x3FA3E3
Main video:             0x3FA700–0x3FFC7F
Main sound/PWM:         0x3FFD00–0x3FFFE3
```

The sound/PWM ranges include the shared disk-speed bytes used by the floppy subsystem.

The `platform macse` line is required. Without it, the Mac SE platform initialization is not selected and floppy testing is invalid.

## Build on the Raspberry Pi

Use the same 32-bit Raspberry Pi OS/native environment as the working artifact. Copy the source tree to the Pi, then run:

```bash
make PLATFORM=PI3_BULLSEYE -j1
```

`-j1` is intentional because the Musashi generated-source step is not safe to parallelize in this tree.

The resulting executable is:

```text
./emulator
```

Rename or copy it as desired:

```bash
cp emulator /home/pi/pistorm_macse_physical_write_filter
sha256sum /home/pi/pistorm_macse_physical_write_filter
```

The build should produce the recorded hash only when the same source revision, compiler, flags, generated Musashi files, and dependencies are used. Always verify the hash rather than assuming it matches.

## Install and run on the PiStorm SE

Copy the editable example configuration and change the ROM path if necessary:

```bash
cp macse-4mb-physical-write-filter.cfg.example \
   /home/pi/macse-4mb-physical-write-filter.cfg
```

The configuration references a Pi-local ROM path:

```text
/home/pi/pistorm_latest_09172026/macse.rom
```

Do not commit the ROM image. Confirm that the ROM exists locally on the Pi, then run:

```bash
sudo /home/pi/pistorm_macse_physical_write_filter \
  --config /home/pi/macse-4mb-physical-write-filter.cfg
```

The emulator requires the PiStorm hardware and the appropriate GPIO permissions. Hardware boot, floppy operation, video refresh, sound, and benchmark behavior must be verified on the target; a successful compile is not a hardware-boot claim.

## Editing the physical-write policy

Each `physwrite` line enables one inclusive-start/exclusive-end range. `size=1M` means one megabyte, so:

```text
physwrite address=0x100000 size=1M
```

covers:

```text
0x100000–0x1FFFFF
```

The parser accepts up to eight ranges. Keep ranges sorted by starting address because the local-write complement is built in configuration order.

Removing every `physwrite` line disables the filter and restores baseline-compatible physical-write behavior.

Do not add arbitrary RAM ranges as a permanent solution without reproducing the behavior and narrowing the dependency. In particular, the `0x100000–0x1FFFFF` range is retained because it was required empirically for reliable floppy boot, not because Apple documentation identifies it as a fixed floppy buffer.

## Files changed

- `config_file/config_file.h` — configuration fields and `physwrite` item.
- `config_file/config_file.c` — parser for editable physical-write ranges.
- `m68k.h` — local write-range API.
- `m68kcpu.c` — local write-range registration.
- `platforms/macse/macse-platform.c` — registers local write ranges covering RAM outside configured physical ranges.
- `emulator.c` — filtered physical-write helper and per-access handling.
- `macse-4mb-physical-write-filter.cfg.example` — working 4 MB configuration example.

## Safety and scope

This repository contains source and documentation only. It does not contain:

- Macintosh ROM images;
- disk images;
- generated emulator binaries;
- build objects;
- credentials, tokens, or SSH keys.

The experiment does not modify the FPGA gateware, implement `/DTACK` polling, emulate the IWM, or relocate floppy traffic to a Pi-side disk cache.

## License

The source retains the upstream project licensing files. See `LICENSE` and individual source-file headers.
