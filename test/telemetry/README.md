# Runtime telemetry target-hardware checklist

1. Request `GET /api/v1/system/telemetry` without a bearer token and confirm
   it returns `401 invalid_access_token`.
2. Request it with user and administrator access tokens and confirm both return
   `200` with `dynamic_allocation=false`.
3. Confirm `uptime_ms` increases and all nine task `stack_min_free_bytes`
   values are nonzero and never increase during one boot.
4. Complete a TLS 1.3 failure callback, then confirm the callback task retains
   safe stack headroom and one delivery counter increments. Confirm the last
   transport, HTTP, elapsed-time, and detail values match the serial result.
5. Return a 2xx response from the callback receiver and confirm
   `delivery_success_count` increments. Return a non-2xx response or cause a
   transport failure and confirm `delivery_failure_count` increments.
6. Hold callback delivery long enough to overflow the three-entry queue.
   Confirm `queue_depth` never exceeds three and `dropped_count` increments
   once for each displaced oldest event.
7. Restart the MCU and confirm callback counters and last-result fields reset.
