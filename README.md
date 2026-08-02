# STM32F767 Health Check

Firmware for an STM32F767ZI-based network health-check device. This repository
is the STM32F7 port of the working STM32F407 health checker and is intended to
retain the same external behavior while taking advantage of the Cortex-M7 and
the F767 memory layout.

The port is currently at its platform-baseline stage. It contains STM32F767
clock and MPU setup, FreeRTOS, a native lwIP OS adaptation, low-level
Ethernet/LAN8742 initialization, and a heartbeat service. HTTPS checks,
persistent NOR Flash storage, time synchronization, sensors, alarms, and the
management API remain to be ported and verified on this MCU.

Project documentation:

- [Porting and operational use cases](USECASES.md)
- [Development rules](docs/DEVELOPMENT_RULES.md)
- [Naming conventions](docs/NAMING_CONVENTIONS.md)
- [Supplying ignored source trees](docs/IGNORED_SOURCES.md)
- [Initial port review and blockers](docs/PORTING_REVIEW.md)
- [Hardware connections](docs/HARDWARE.md)

## Current baseline

- STM32F767ZI Cortex-M7 running at 216 MHz
- instruction and data caches enabled
- FreeRTOS Kernel 11.1.0 using the Cortex-M7 GCC port
- statically allocated native FreeRTOS adaptation for lwIP
- low-level Ethernet MAC and LAN8742 PHY initialization
- non-cacheable, MPU-aligned Ethernet DMA memory
- statically allocated heartbeat task
- GNU Arm Embedded Makefile build

## F407 reference assets

The `test/` and `tools/` directories are carried from the STM32F407 project as
the compatibility target for the port:

- `test/dnsmasq/` supplies Linux and macOS DHCP/DNS test-network helpers;
- `test/postman/` contains the management API acceptance collection;
- `tools/` contains credential and verifier preparation utilities.

The Postman collection and TLS-related tools become applicable as their
corresponding firmware services are ported. Their presence does not mean those
services are available in the current baseline.

## Build

Install the ignored vendor sources described in
[docs/IGNORED_SOURCES.md](docs/IGNORED_SOURCES.md), then run:

```sh
make clean
make -j4
```

Build products are written to `build/`. Hardware upload and debugger settings
are maintainer-specific and remain in `.vscode/`; contributors must ask before
changing those files.

The current baseline completes a clean build. See
[the initial port review](docs/PORTING_REVIEW.md) for corrected findings and
the remaining network-integration work.

## Porting constraints

Code copied from STM32F407 must be reviewed rather than transferred blindly.
The F767 differs in cache coherency, DMA-visible memory, interrupt behavior,
clocking, and linker layout. Ethernet DMA descriptors and packet buffers need
particular attention because D-cache can make otherwise correct F407 code fail
on Cortex-M7.

Copyright © 2017-2026 Dmitry Slobodchikov.
