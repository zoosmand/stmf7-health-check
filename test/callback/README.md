# Outbound callback regression checklist

1. After factory reset, confirm `GET /api/v1/callback/config` reports callback
   delivery disabled and the default `callback.invalid:443/` target.
2. Install the callback server's CA as a trust anchor and enable POST delivery.
   Confirm successful checks do not invoke it and each failed check produces
   JSON containing `sequence`, `timestamp`, `resource_index`, `status`,
   `stage`, `http_status`, `elapsed_ms`, and `detail`.
3. Select GET delivery and confirm the same eight values arrive as query
   parameters, including when the configured path already contains a query.
4. Confirm callback `status` is always `fail`; transport failures have a
   descriptive `stage`, while unexpected HTTP status codes have stage `ok`.
   Confirm absent HTTP responses report `http_status=0` and that `sequence`
   and `timestamp` match the persisted log entry.
5. Delay callback responses and confirm failure enqueueing remains immediate.
   Confirm later health checks and management requests resume after the shared
   TLS operation completes, and that queue overflow retains the newest three
   pending failures.
6. Keep a management TLS request active while a callback is pending. Confirm
   both operations finish without concurrent Mbed TLS allocator access.
7. Try invalid method, host, port, path, trust-anchor ID, and oversized encoded
   request combinations; confirm the update is rejected without changing the
   persisted configuration.
8. Enable callback delivery and confirm its trust anchor cannot be deleted or
   reset. Disable delivery and confirm removal is allowed when unused elsewhere.
9. Reboot after configuration changes and confirm the newest valid A/B snapshot
   is restored. Interrupt a factory reset and confirm recovery erases callback
   configuration before the device resumes normal startup.
