# Design: Boot-time HTTP Server domain

Date: 2026-11-06
Status: Proposed

## Goal

Make `test.net.WebServer` listen on port 80 after boot so that an HTTP client can connect (`curl http://<host>:80/` returns a response). Today nothing starts `WebServer` — every reference to `test.net.WebServer.main(...)` (in `test/APP/src/AI/AI.java` and `test/HCI/test/test/AI.java`) is commented out, so port 80 is never bound even when NET is up.

## Scope

- **In scope:**
  - Add a boot-time domain (`WebServerBoot`) that starts `test.net.WebServer` with no arguments (default port 80, no filesystem).
  - Wire the domain start into boot after the `NET` portal is available.
  - Make the `accept()` loop resilient: log and continue on `IOException` instead of killing the whole server (one bad client must not take port 80 offline).
- **Out of scope:** filesystem-backed file serving (`-fs`), HTTPS, virtual hosts, keepalive, POST body handling. These are YAGNI for "can't connect at all."

## Architecture

```
Boot
  └─ NET portal registered (TCP stack up)
       └─ WebServerBoot domain starts
            └─ test.net.WebServer.main(new String[0])
                 ├─ ServerSocket(80) bound to NET
                 └─ accept() loop
                      └─ TCPWebWorker.processRequest(sock, null)
                           ├─ parse HTTP request line
                           └─ serve hardcoded index HTML (HTTP/1.0 200 OK)
```

## Components

| Component | Responsibility | Depends on |
|---|---|---|
| `WebServerBoot` domain | Launch `test/net/WebServer` via `DomainStarter.createDomain` | `NET` portal |
| `test.net.WebServer` | Bind port 80, accept connections, spawn workers | `NET` portal (via `ServerSocket`) |
| `test.net.TCPWebWorker` | Parse request, serve hardcoded page | `Socket`, no FS |

`WebServer.java` needs no functional change — its no-args constructor already binds port 80 and serves hardcoded pages. `TCPWebWorker.java` needs no change for the no-FS case either (the hardcoded `default` branch is used).

## Data flow

1. OS boot → `NET` portal registered (TCP stack operational).
2. Boot integration starts `WebServerBoot` domain.
3. Domain runs `WebServer.main(new String[0])`:
   - `ServerSocket(80)` created via `net.getTCPSocket(80, ...)`.
   - `accept()` loop blocks waiting for connections.
4. Client connects (e.g. `curl`) → `TCPSocket.accept()` returns `Socket`.
5. `TCPWebWorker.processRequest(sock, null)`:
   - Reads HTTP request lines via `DataInputStream.readLine()` until blank line.
   - Extracts the requested path (ignored — no FS).
   - Writes `HTTP/1.0 200 OK` header + hardcoded index HTML.
   - `sock.close()`.
6. Loop returns to step 4 for the next connection.

## Error handling

- `NET` portal not yet available at boot → `waitUntilPortalAvailable(naming, "NET")` blocks until `NET` appears; domain start is deferred, not failed.
- `accept()` throws `IOException` (e.g. malformed client) → log via `Debug.out.println` and **continue** the loop (fixes the current `break` that kills port 80 on one bad connection).
- Worker thread exception → caught and logged (fixes the currently-swallowed `//throw new Error();`).
- No filesystem → `TCPWebWorker` never calls `readFile`, so no FS-dependent code path is exercised.

## Testing

1. Boot the OS in QEMU/simulator.
2. Confirm NET is up and an IP is assigned.
3. `curl http://<host>:80/` from a client on the same network.
4. Expect `HTTP/1.0 200 OK` followed by the hardcoded index HTML body.
5. Repeat with multiple sequential connections to confirm port 80 stays alive.

## Files changed

- `test/NET/src/test/net/WebServer.java` — `accept()` loop: log + continue on `IOException` instead of `break`; worker exceptions logged.
- New: boot integration that starts `WebServerBoot` domain after `NET` portal available (implementation plan will locate the exact boot hook).
