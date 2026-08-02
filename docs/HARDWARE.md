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
