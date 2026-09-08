# iPhone MEMS Tools (Foundation)

This folder contains an initial iOS shell for ECU MEMS Manager, aligned with the desktop workflows:

- Connection / disconnection
- Live summary gauges/values
- Diagnostic analysis stream
- Logging/export area
- Settings area

## MVP scope

The current iOS foundation focuses on:

1. App shell and UX flow parity with desktop tabs.
2. Shared core extraction from desktop C++ for reusable data and rules.
3. Transport abstraction (`MemsTransport`) for iPhone-compatible adapters.

## iPhone communication constraints

iOS does not support the desktop serial stack directly. The transport layer must target iOS-compatible links/adapters (for example BLE-capable or MFi-compliant interfaces), then feed normalized snapshots into the shared core.

## Staged validation path

1. Simulator validation for UI and state transitions.
2. On-device adapter integration checks.
3. End-to-end validation on real ECU sessions.
