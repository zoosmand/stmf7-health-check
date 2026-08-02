# Use cases

This document is the operational acceptance reference for functionality being
ported from the STM32F407 health checker. The current STM32F767 baseline does
not yet expose the HTTPS management API described below.

## Add a Let's Encrypt trust anchor

Use this procedure after the TLS management API is ported, when an HTTPS
resource presents a certificate chain rooted at ISRG Root X1 and fails at the
certificate-validation stage.

Download the ISRG Root X1 certificate in DER format and create a PEM copy for
inspection:

```sh
curl http://x1.i.lencr.org/ --output ./ISRG_Root_X1.der && \
openssl x509 -in ISRG_Root_X1.der -out ISRG_Root_X1.pem \
  -inform der -outform pem
```

Because the download uses HTTP, verify the certificate subject, issuer,
validity, CA constraints, and SHA-256 fingerprint against an authoritative
Let's Encrypt source before trusting it:

```sh
openssl x509 -in ISRG_Root_X1.pem -noout \
  -subject -issuer -dates -fingerprint -sha256 -text
```

Upload the original DER certificate with an administrator bearer token:

```sh
curl -sk -X POST https://<device>/api/v1/trust-anchors \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/octet-stream" \
  --data-binary @ISRG_Root_X1.der
```

Assign the returned anchor ID to the intended resource, then restart the board
and verify that both the trust anchor and resource association persist.
