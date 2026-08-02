# Supplying ignored source trees

The repository does not track the STM32 vendor driver distribution. This keeps
project history focused on project-owned firmware while allowing each checkout
to use the matching, licensed STMicroelectronics package.

## Required local sources

Before building, provide these paths from a known-good STM32F767 package:

```text
Drivers/CMSIS/
Middlewares/Third_Party/MbedTLS/
```

The checkout tracks the FreeRTOS kernel and the project-specific lwIP tree. Do
not replace them with arbitrary releases. Supply Mbed TLS 3.6.7 at the exact
path above; the Makefile and `TLS/Inc/health_checker_mbedtls_config.h` select
the embedded subset used by the TLS 1.3 checker. The known-good source tree can
also be copied from the neighboring STM32F407 health-check project.

## Prepare a fork

1. Clone the fork:

   ```sh
   git clone <fork-url>
   cd stmf7-health-check
   ```

2. Copy the compatible STM32F7 CMSIS source tree into `Drivers/CMSIS/` and
   Mbed TLS 3.6.7 into `Middlewares/Third_Party/MbedTLS/`.

3. Generate the ignored management-server certificate, key, and embedded
   credential header:

   ```sh
   tools/generate_server_certificate.sh health-check.local 192.168.0.50
   ```

4. Confirm that the device headers include STM32F767 support and that the local
   toolchain provides `arm-none-eabi-gcc`.

5. Build from the repository root:

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
