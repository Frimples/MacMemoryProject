# Mac Memory Project

A separate test repository for giving a PiStorm-accelerated Macintosh SE more
application memory through Pi-backed RAM and 68030/68851 PMMU-compatible
translation.

This repository is experimental. It does not modify the production PiStorm
repository or the verified `macse-warpspeed` build.

## Goal

Test whether a Macintosh SE can use Pi-side memory for application allocations
while preserving the stock SE hardware-visible behavior:

- 68000-compatible SE ROM boot behavior;
- 24-bit Macintosh addressing compatibility;
- 4 MB physical SE RAM and BBU video/sound access;
- IWM, VIA, ADB, SCSI, sound, and interrupts;
- PiStorm fast RAM for the normal address space;
- additional Pi-backed memory available only through a PMMU-capable path.

## Current hypothesis

The most realistic path is to emulate a full 68030 with PMMU support and expose
additional Pi memory through page translation. A simple `map type=ram` range is
not expected to increase the Macintosh application heap because the classic Mac
Memory Manager will not discover an arbitrary emulator-only range.

Connectix Compact Virtual is a compatibility reference and possible guest-side
test, not an assumed dependency. The original software expects a compatible
68030 accelerator and accelerator RAM. We will not redistribute proprietary
Connectix binaries.

## Validation reference

Snow is an independent open-source classic Macintosh emulator that supports the
SE family, 68030, 68851 PMMU, and detailed debugging. It may be used as a
behavioral and guest-compatibility reference. Snow source is not copied into
this repository.

- https://github.com/twvd/snow
- https://snowemu.com/

## Planned test layers

1. Host-side PMMU translation tests.
2. 68030 PMMU instruction and exception tests.
3. Synthetic Pi-backed extended-memory reads and writes.
4. System 6.0.8 or System 7.1 memory-manager tests.
5. Compact Virtual detection and allocation tests.
6. Snow control run.
7. MAME control run.
8. PiStorm emulator run.
9. Real Macintosh SE hardware test.

A passing earlier layer is not evidence that a later layer works.

## Rules

- Keep all work in this repository or its own worktrees.
- Do not alter `/home/xander/macSE_research/macse-warpspeed/` without explicit approval.
- Do not use, copy, rename, compile, or retain the upstream `mac68k` platform.
- Do not commit ROM images, disk images, WADs, binaries, object files, credentials,
  or generated test artifacts.
- Treat Snow and MAME as behavioral references; implement PiStorm behavior
  independently.
- Preserve the verified `wtcram` video/sound configuration as a control.
- Do not claim application-visible extended RAM until a guest allocation test
  succeeds and survives a reboot/control comparison.

## Status

Repository initialization. No PMMU or memory implementation is claimed yet.
The first implementation should begin with host-side translation tests before
modifying the PiStorm emulator.
