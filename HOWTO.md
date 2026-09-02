# Configure a new device

A fresh device has no HTTPS resources or outbound trust anchors. Complete the
following steps after flashing the firmware. The examples require `curl`,
`jq`, and `openssl`.

## 1. Find and verify the device

Read the assigned DHCP or fallback address from the serial console, then set
the API URL. The compiled management certificate is self-signed, so these
initial examples use `-k`. Use normal certificate verification after installing
a deployment server certificate.

```sh
DEVICE_URL=https://172.18.10.155
curl -sk "$DEVICE_URL/health" | jq
```

The device is ready for configuration when `/health` reports HTTP `200` and
`"status":"ok"`.

## 2. Obtain an administrator token

Use the master password compiled into this firmware build:

```sh
MASTER_PASSWORD='replace-with-the-master-password'

TOKEN=$(curl -sk -X POST "$DEVICE_URL/api/v1/auth/token" \
  -H 'Content-Type: application/json' \
  --data "{\"username\":\"master\",\"password\":\"$MASTER_PASSWORD\"}" \
  | jq -r '.access_token')
```

Confirm that `$TOKEN` is neither empty nor `null` before continuing. A new
login invalidates the previous token for the same account.

## 3. Prepare and upload a root CA

Obtain the root CA that validates the intended HTTPS resource. For a resource
rooted at ISRG Root X1, one possible preparation command is:

```sh
curl http://x1.i.lencr.org/ --output ./ISRG_Root_X1.der && \
openssl x509 -in ISRG_Root_X1.der -out ISRG_Root_X1.pem \
  -inform der -outform pem
```

Verify the subject, issuer, validity, CA constraints, and SHA-256 fingerprint
against an authoritative source before uploading a certificate obtained over
HTTP:

```sh
openssl x509 -in ISRG_Root_X1.pem -noout \
  -subject -issuer -dates -fingerprint -sha256 -text
```

Upload the DER file and retain the returned ID:

```sh
ANCHOR_ID=$(curl -sk -X POST "$DEVICE_URL/api/v1/trust-anchors" \
  -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/octet-stream' \
  --data-binary @ISRG_Root_X1.der \
  | jq -r '.id')
```

The device supports six uploaded anchors with IDs from `1` through `6`.

## 4. Create the first resource

Specify the hostname without a URL scheme and use a path beginning with `/`:

```sh
curl -sk -X POST "$DEVICE_URL/api/v1/health-check/resources" \
  -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/json' \
  --data "{\"host\":\"example.com\",\"port\":443,\"path\":\"/\",\"enabled\":true,\"trust_anchor_id\":$ANCHOR_ID}" \
  | jq
```

Replace `example.com` with the target whose certificate chain uses the uploaded
root CA. The checker uses TLS 1.3 and HTTP `HEAD`; only status `200` is healthy.

## 5. Verify operation

```sh
curl -sk "$DEVICE_URL/api/v1/trust-anchors" \
  -H "Authorization: Bearer $TOKEN" | jq

curl -sk "$DEVICE_URL/api/v1/health-check/config" \
  -H "Authorization: Bearer $TOKEN" | jq

curl -sk "$DEVICE_URL/api/v1/rtc" \
  -H "Authorization: Bearer $TOKEN" | jq

curl -sk "$DEVICE_URL/api/v1/health-check/logs" \
  -H "Authorization: Bearer $TOKEN" | jq
```

Checks run every 60 seconds by default. Configuration and results persist in
the W25Q64 NOR Flash across resets.

Delete configured resources before resetting the complete trust store. The API
returns HTTP `409` if any resource still references an anchor.

## Restore factory state

Hold the blue B1 user button for at least five seconds. Release it after the
two-tone acknowledgement. The device erases all API-created users, sessions,
resources, trust anchors, uploaded server credentials, and logs, then restarts.
Short presses are ignored. After restart, repeat this guide using the compiled
master password and recovery certificate.

Do not deliberately remove power during the erase. If power is interrupted,
the persistent reset marker makes the firmware finish the erase automatically
on its next boot before the management services start.
