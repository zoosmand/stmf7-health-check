# Initial STM32F767 port review

This review records the F767 baseline refactoring performed before application
services are ported from the STM32F407 health checker.

## Verdict

The repository now provides a cleanly building hardware and FreeRTOS baseline.
Its project-owned startup, GPIO, heartbeat, Ethernet setup, and lwIP OS adapter
have consistent interfaces and documented failure behavior. The later network
baseline adds the lwIP frame adapter, link monitoring, DHCP, and static
fallback. Application protocols remain future work.

## Corrected findings

- Replaced the incompatible CMSIS-RTOS/lwIP wrapper with a statically allocated
  native FreeRTOS adaptation.
- Corrected MDIO and Ethernet DMA polling directions and added bounded startup
  timeouts.
- Applied the LAN8742 autonegotiated speed and duplex instead of assuming
  100-Mbit full duplex.
- Replaced the inverted Ethernet-ready result with a typed status returned by
  `Board_InitEthernet()` and reported by `main()`.
- Reserved an MPU-aligned 32 KiB SRAM region at `0x20078000` for Ethernet DMA
  descriptors and buffers, and configured it as non-cacheable before enabling
  the Cortex-M7 data cache.
- Preserved an already valid LSE-backed backup domain instead of resetting RTC
  state on every boot.
- Added bounded clock and oscillator waits and completed the 216 MHz overdrive
  and 48 MHz PLL configuration.
- Replaced broad build wildcards with a reviewed IPv4/lwIP/FreeRTOS source set.
  Unused IPv6, PPP, legacy crypto, and vendor driver files remain in the tree but
  are not compiled.
- Removed the undefined display output selection and corrected USART diagnostic
  output to wait on transmitter status rather than a request-register bit.
- Aligned project-owned names, headers, public documentation, and structures
  with the repository conventions.

## Remaining port work

- Port RTC/NTP, watchdog, sensors, and diagnostic services.
- Port Mbed TLS, HTTPS checks, persistent configuration/logging, the management
  API, and alarms in individually testable stages.

## Verification

- `make clean && make -j4` completes without compiler or linker warnings.
- Resulting baseline size: 9,708 bytes text, 124 bytes initialized data, and
  32,756 bytes BSS, including the reserved Ethernet DMA buffers.
- Host-side Python, shell, Postman JSON, and whitespace checks pass.
- Target-device behavior still requires maintainer testing.
