# Factory-reset target-hardware checklist

1. Tap, bounce, and hold B1 for less than ten seconds. Confirm no reset warning
   occurs and persistent configuration remains unchanged.
2. Hold B1 continuously for ten seconds. Confirm exactly five long warning
   beeps play before the ten-second cancellation window starts.
3. Release B1 and double-click it within 600 ms during the window. Confirm
   exactly three long acknowledgement beeps play, the reset is cancelled, and
   all persistent configuration remains available.
4. Try one click, two clicks spaced more than 600 ms apart, and noisy button
   transitions. Confirm none cancels the reset.
5. Allow the cancellation window to expire. Confirm the device restarts with
   no health-check resources or log entries, callback configuration at its
   default, no persistent users or trust anchors, and the compiled factory TLS
   certificate and private key active.
6. Remove power after the cancellation window expires while erasure is in
   progress. Restore power and confirm startup completes the marked reset
   before loading any persistent store.
7. Remove power during the warning or cancellation window. Confirm the device
   starts normally without performing a factory reset because no durable marker
   had yet been written.
