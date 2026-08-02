# Supplying ignored source trees

The repository does not track the STM32 vendor driver distribution. This keeps
project history focused on project-owned firmware while allowing each checkout
to use the matching, licensed STMicroelectronics package.

## Required local sources

Before building, provide these paths from a known-good STM32F767 package:

```text
Drivers/CMSIS/
```

The checkout currently tracks the FreeRTOS kernel and the project-specific
lwIP tree. Do not replace them with arbitrary releases: the firmware integration
depends on their checked-in versions and configuration.

Future health-check stages will add Mbed TLS. When it is introduced, use the
exact submodule revision recorded by this repository and initialize nested
submodules with:

```sh
git submodule update --init --recursive
```

## Prepare a fork

1. Clone the fork and initialize recorded submodules:

   ```sh
   git clone <fork-url>
   cd stmf7-health-check
   git submodule update --init --recursive
   ```

2. Copy the compatible STM32F7 CMSIS source tree into `Drivers/CMSIS/`.

3. Confirm that the device headers include STM32F767 support and that the local
   toolchain provides `arm-none-eabi-gcc`.

4. Build from the repository root:

   ```sh
   make clean
   make -j4
   ```

The Makefile compiles only explicitly selected project and middleware sources.
Do not delete unused files from an installed vendor distribution merely to
reduce the firmware build.

## Sharing a fork

Ignored paths are not added by ordinary `git add`. That is intentional. Record
the vendor package name and version used by the fork so collaborators can
install the same dependency locally.

If a fork owner chooses to publish vendor sources, first verify licensing and
provenance, then use a dedicated submodule or narrowly change `.gitignore`.
Never force-add an entire ignored tree without reviewing its contents. Build
artifacts, logs, editor settings, credentials, private keys, and generated
secret-bearing files must remain untracked.
