# Hardware connections

This document records the wiring used by the STM32F767 health-check prototype.
Keep it synchronized with the board initialization code.

## W25Q64 NOR Flash

The external Winbond W25Q64 is connected to SPI1:

| Signal | STM32F767 pin | Configuration |
|--------|---------------|---------------|
| SCK | PB3 | SPI1 alternate function 5 |
| MISO | PB4 | SPI1 alternate function 5 |
| MOSI | PB5 | SPI1 alternate function 5 |
| NSS/CS | PA4 | software-controlled GPIO output |

PA4 should be driven high before SPI1 is enabled so the Flash remains
deselected during startup. Use software-controlled chip select rather than the
SPI peripheral's hardware NSS mode; this preserves transaction boundaries
required by W25Q commands and makes the driver easier to reuse.

PB3 and PB4 overlap full-JTAG signals on STM32F767. They remain available when
the board uses the two-wire SWD debug interface, but firmware and debugger
configuration must not enable full JTAG while SPI1 is in use.

SPI1 operates as an 8-bit mode-0 master at 27 MHz with software-controlled
chip select. Startup performs only a JEDEC identity read and expects
`EF 40 17`; it does not erase or program Flash contents. The driver provides
bounded blocking read, 4 KiB sector erase, and page-aware program operations.
Access is serialized by a statically allocated FreeRTOS mutex.

## Factory-reset button

The NUCLEO-F767ZI B1 user button uses its default active-high `PC13` connection.
Firmware configures the pin as an input without an internal pull resistor and
polls it from a FreeRTOS service. Holding B1 for five seconds initiates factory
reset; short presses are ignored. This assumes the board's default B1 solder-
bridge configuration remains unchanged.

## Diagnostic console

The ST-LINK virtual COM port is connected to USART3 and is the standard
`printf()` destination on every build host:

| Signal | STM32F767 pin | Configuration |
|--------|---------------|---------------|
| VCP TX | PD8 | USART3 alternate function 7 |
| VCP RX | PD9 | USART3 alternate function 7 |

Serial settings are **115200 baud, 8 data bits, no parity, 1 stop bit**.

## Buzzer alert

A passive buzzer is driven through a 2N2222 transistor rather than directly
from the GPIO:

| Signal | STM32F767 pin | Configuration |
|--------|---------------|---------------|
| Buzzer PWM | PA3 | TIM2 channel 4, alternate function 1, exposed at `CN8/A0` |

The MCU pin drives the transistor base through a series resistor; the buzzer
itself is powered through the transistor's collector/emitter path, never by
the GPIO pin directly. TIM2 channel 4 produces a fixed 2.5 kHz square wave at
roughly 50% duty. Outside an active tone, `PA3` is reconfigured as a plain low
GPIO output and TIM2's counter and channel output are both disabled, because
TIM2 has no output idle-state control (that feature exists only on
advanced-control timers) and the pin state while the channel is merely
disabled is otherwise unspecified.

## Ethernet identity

The firmware derives the Ethernet MAC address from the STM32F767's factory
96-bit unique device ID. The derived address is stable for a given MCU and is
marked as locally administered and unicast; no globally assigned vendor OUI is
claimed. Startup output prints the resulting address for DHCP reservations and
device identification.
