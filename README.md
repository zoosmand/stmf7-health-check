# STM32F767 Health Check

Firmware for an STM32F767ZI-based network health-check device. This repository
is the STM32F7 port of the working STM32F407 health checker and is intended to
retain the same external behavior while taking advantage of the Cortex-M7 and
the F767 memory layout.

The port contains STM32F767 clock and MPU setup, FreeRTOS, a native lwIP OS
adaptation, Ethernet/LAN8742 networking with DHCP and static fallback, W25Q64
NOR Flash access, a heartbeat service, SNTP time synchronization, periodic
authenticated HTTPS resource checks, an audible buzzer alert on check failure,
and an authenticated HTTPS management API. Sensors and alarms remain to be
ported and verified on this MCU.

Project documentation:

- [Configure a new device](HOWTO.md)
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
- Ethernet MAC/DMA and LAN8742 link monitoring
- non-cacheable, MPU-aligned Ethernet DMA memory
- lwIP IPv4 frame adapter with DHCP and static fallback
- W25Q64 SPI NOR Flash driver
- SNTP synchronization with `pool.ntp.org`
- Mbed TLS 3.6.7 with hardware-RNG entropy and TLS 1.3-only policy
- persistent HTTPS resource configuration and wear-aware result log
- periodic authenticated HTTP `HEAD` checks
- asynchronous configurable HTTPS callbacks for failed checks
- authenticated runtime telemetry for task stacks and callback delivery
- TIM2/PA3 buzzer alert on failed resource checks
- power-loss-safe factory reset by holding the B1 user button
- TLS 1.3 management API with bearer-token authentication
- statically allocated heartbeat task
- GNU Arm Embedded Makefile build

## F407 reference assets

The `test/` and `tools/` directories are carried from the STM32F407 project as
the compatibility target for the port:

- `test/dnsmasq/` supplies Linux and macOS DHCP/DNS test-network helpers;
- `test/postman/` contains the management API acceptance collection;
- `tools/` contains credential and verifier preparation utilities.

The TLS verifier tools apply to the resource checker and management server.
The Postman collection is the end-to-end management API acceptance suite.

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
remaining service-porting work.

## Network configuration

With an active Ethernet link, the firmware requests configuration through
DHCP. If no lease is available after 10 seconds, it applies:

- IP: `192.168.0.50`
- mask: `255.255.255.0`
- gateway: `192.168.0.1`
- DNS: `8.8.8.8`

DHCP remains eligible to replace fallback settings after link or server
recovery. Each configuration change is reported through USART3:

```text
ETH configuration: DHCP
ETH IP: 172.18.10.129
ETH MASK: 255.255.255.0
ETH GATE: 172.18.10.1
ETH DNS: 172.18.10.1
```

## HTTPS health checking

After the network has a usable IPv4 address and SNTP supplies trusted UTC, the
health-check task prints the synchronized UTC time once, then checks every
enabled resource sequentially. A fresh device has no resources or trust
anchors; upload a CA certificate and create a resource through the management
API before checks can run.

The request uses TLS 1.3 and HTTP `HEAD`. Certificate chain, hostname, and
validity dates are verified against the resource's selected trust anchor. A
resource is healthy only when the TLS transaction succeeds and the server
returns HTTP status `200`; redirects and all other statuses are failures.

Successful output has this form:

```text
RTC: 2026-08-03T09:15:42Z
HTTPS check: https://example.com/
TLS: TLSv1.3, <cipher suite>, certificate valid
HTTP HEAD: 200, <elapsed> ms
Resource health: OK
```

Failure output reports the last completed transport stage:

```text
HTTPS failure: stage=<stage>, detail=<detail>, <elapsed> ms
Resource health: FAILED
```

| Stage | Meaning |
| ---: | --- |
| 1 | DNS resolution failed. |
| 2 | TCP connection failed. |
| 3 | TLS configuration or allocation failed. |
| 4 | Certificate parsing or validation failed. |
| 5 | TLS handshake failed for a reason other than certificate validation. |
| 6 | Encrypted request transmission failed. |
| 7 | The HTTP status line was missing or malformed. |

`detail` is an lwIP error, a negated socket `errno`, or an Mbed TLS error code,
depending on the stage. Checks run every 60 seconds by default. Configuration
supports six resources and periods from 60 through 1800 seconds; the management
API provides persistent mutation. The trust store supports six uploaded CA
certificates; firmware contains no outbound factory trust anchor.

Configuration is stored as alternating transactional snapshots in W25Q64
sectors 3 and 4 from the top of the device. Results use an append-only ring in
sectors 7 and 8 from the top. Trust anchors use two four-sector banks in sectors
19 through 26. Previous three-sector and two-sector bank locations remain
reserved for migration. Persistent allocations must not be reordered without
a migration.

## Buzzer alert

A passive buzzer on `PA3`/`TIM2_CH4` (connector `CN8/A0`) sounds a bounded
three-beep pattern whenever a resource check fails; successful checks stay
silent. A single short tone plays once at startup, so wiring can be verified
without waiting for a failure. The buzzer task serializes repeated alerts
through a one-entry static queue: a pattern already queued or playing absorbs
further failures instead of queuing a backlog. TIM2's counter and channel
output stay disabled whenever no tone is playing, and `PA3` is held as a plain
low output outside an active tone, so the transistor base is never left
floating. Factory reset uses five long warning beeps; a successful cancellation
uses three long acknowledgement beeps.

## Factory reset

Hold the NUCLEO B1 user button continuously for ten seconds to request a
factory reset. Short or bouncing presses have no effect. Five warning beeps
then play, followed by a ten-second cancellation window. Button clicks during
the warning sequence are ignored. After the fifth beep finishes, release B1
and double-click it within 600 ms to cancel; three acknowledgement beeps
confirm the cancellation and normal monitoring resumes.

If the cancellation window expires, the firmware stores a durable reset marker,
then erases resources and their period, callback configuration, trust anchors,
users and sessions, uploaded management credentials, and health-check logs
from W25Q64 before restarting. Button press and release edges use 60 ms
debouncing for both the long hold and double-click gesture.

The compiled master-password verifier and compiled recovery server certificate
and key remain available. A dedicated Flash marker is erased last, so startup
automatically completes a reset interrupted by power loss before any persistent
store is loaded.
Five short tones report a Flash failure. A marker-write failure cancels the
reset without claiming success; an erase failure restarts with the marker still
present so startup can retry safely.

## Management API

### Outbound failure callbacks

Callback delivery is disabled by default and initially targets
`https://callback.invalid/`. A dedicated static FreeRTOS task consumes a
three-entry queue, so adding a failed result never blocks the health-check
producer. Delivery shares the serialized TLS platform with health checks and
the management API, so a slow callback can delay later TLS operations. If the
queue fills, the oldest pending result is discarded and the newest result is
retained.

Only failed health checks trigger a callback. Each failure is persisted before
it is queued; if the log write cannot be verified, the callback is skipped.
POST sends the exact persisted diagnostic values plus a readable transport
stage:

```json
{"sequence":8285,"timestamp":1790498283,"resource_index":2,"status":"fail","stage":"ok","http_status":503,"elapsed_ms":1260,"detail":1001}
```

GET appends the same eight values as query parameters. `resource_index` is the
zero-based configured slot. `stage` is `ok`, `dns_error`, `connect_error`,
`config_error`, `certificate_error`, `handshake_error`, `io_error`, or
`protocol_error`. An unexpected HTTP response such as 503 has stage `ok`
because the transport itself completed successfully. Callback configuration
uses two transactional 4 KiB NOR sectors. Runtime resources are a three-entry
static queue, an 8 KiB callback-task stack, and bounded 192-byte body,
256-byte request-target, and 512-byte HTTP request buffers.

Read the current configuration:

```http
GET /api/v1/callback/config
Authorization: Bearer <access-token>
```

Partially update it with an administrator token:

```http
PUT /api/v1/callback/config
Authorization: Bearer <access-token>
Content-Type: application/json

{"enabled":true,"method":"POST","host":"loopback.intraclear.com","port":443,"path":"/","trust_anchor_id":1}
```

The selected trust anchor cannot be deleted or reset while callback delivery
is enabled. Factory reset erases both callback configuration sectors.

### Runtime telemetry

Any authenticated user can read non-persistent MCU telemetry:

```http
GET /api/v1/system/telemetry
Authorization: Bearer <access-token>
```

The response reports `uptime_ms`, minimum-ever unused stack space in bytes for
each registered application task, and callback queue/delivery counters. A task
stack value of zero means the task was not registered when the snapshot was
taken. Callback delivery succeeds only when its TLS transport completes and
the receiver returns an HTTP status from 200 through 299. The last callback
diagnostic fields remain zero until the first delivery attempt. Counters are
unsigned 32-bit runtime values and reset when the MCU restarts.

`dynamic_allocation` is always `false`: this firmware uses statically allocated
FreeRTOS objects and has no FreeRTOS heap, so free-heap telemetry would be
misleading. CPU utilization and calibrated internal temperature/supply
measurements are not collected.

The device listens on TCP port `443` after networking and TLS storage are
ready. Its compiled recovery certificate is self-signed, so development clients
must explicitly trust it or disable verification only for isolated testing.
Generate a deployment certificate and embedded recovery header with:

```sh
tools/generate_server_certificate.sh health-check.local 192.168.0.50
```

The built-in administrator username is `master`. Firmware stores only a random
salt and PBKDF2-HMAC-SHA-256 verifier. Generate or rotate it with:

```sh
python3 tools/generate_master_verifier.py
```

Rebuild and reflash after changing the compiled master verifier or recovery
certificate. Additional users are stored in NOR flash. Every account has one
session: login or refresh rotates both tokens and invalidates the previous
pair. Access tokens last 15 minutes, refresh tokens seven days, and reset
invalidates all sessions.

| Method | Endpoint | Authorization | Purpose |
| --- | --- | --- | --- |
| `GET`, `HEAD` | `/health` | None | Check API, network, synchronized time, and NOR flash. |
| `POST` | `/api/v1/auth/token` | None | Exchange credentials for an access/refresh pair. |
| `POST` | `/api/v1/auth/refresh` | Refresh token in JSON | Rotate both tokens. |
| `POST` | `/api/v1/auth/revoke` | Bearer | Revoke the current session. |
| `GET` | `/api/v1/rtc` | Any bearer | Return synchronized Unix time and UTC date/time. |
| `GET` | `/api/v1/system/telemetry` | Any bearer | Return uptime, task stack headroom, and callback counters. |
| `GET` | `/api/v1/callback/config` | Any bearer | Read outbound callback configuration. |
| `PUT` | `/api/v1/callback/config` | Administrator bearer | Partially update callback configuration. |
| `GET`, `POST` | `/api/v1/users` | Administrator bearer | List or create users. |
| `PUT`, `DELETE` | `/api/v1/users/{username}` | Administrator bearer | Update or delete a user. |
| `PUT` | `/api/v1/tls/certificate` | Administrator bearer | Stage a DER server certificate. |
| `PUT` | `/api/v1/tls/private-key` | Administrator bearer | Stage a DER server private key. |
| `GET` | `/api/v1/trust-anchors` | Any bearer | List CA anchors. |
| `POST`, `DELETE` | `/api/v1/trust-anchors` | Administrator bearer | Add or reset CA anchors; reset requires no resources. |
| `PUT`, `DELETE` | `/api/v1/trust-anchors/{id}` | Administrator bearer | Replace or delete an anchor. |
| `GET` | `/api/v1/health-check/config` | Any bearer | Read the check period and resources. |
| `PUT` | `/api/v1/health-check/config` | Administrator bearer | Change the check period. |
| `POST` | `/api/v1/health-check/resources` | Administrator bearer | Add one of six resources. |
| `PUT`, `DELETE` | `/api/v1/health-check/resources/{index}` | Administrator bearer | Update or remove a resource. |
| `GET` | `/api/v1/health-check/logs` | Any bearer | Return the 50 newest results. |

Passwords must contain 12 through 128 bytes and usernames at most 24 bytes.
The unauthenticated `/health` endpoint returns HTTP `200` with `status: "ok"`
only when API, network, synchronized time, and flash are operational; otherwise
it returns HTTP `503`. Remote-resource health is reported through the log and
does not affect this device self-check. The response also carries `version`
(from the `.version` file at build time) and `build_date` (UTC, ISO 8601, set
when the firmware was built).

Server credential uploads accept raw DER only: one certificate up to 1152 bytes
and one private key up to 384 bytes. The pair is parsed, matched, and committed
transactionally; subsequent connections use it. The compiled recovery pair is
used whenever no valid flash override exists.

Import `test/postman/STM32_F767_Health_Check_API.postman_collection.json` and
set `baseUrl`, `masterPassword`, `testPassword`, and the three DER file paths.
Postman may require binary upload files to be selected manually. The collection
retains issued tokens and created resource/anchor indices automatically.

## Porting constraints

Code copied from STM32F407 must be reviewed rather than transferred blindly.
The F767 differs in cache coherency, DMA-visible memory, interrupt behavior,
clocking, and linker layout. Ethernet DMA descriptors and packet buffers need
particular attention because D-cache can make otherwise correct F407 code fail
on Cortex-M7.

Copyright © 2017-2026 Dmitry Slobodchikov.
