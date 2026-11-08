# HTTP Server Boot Domain Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `test.net.WebServer` listen on port 80 after boot so an HTTP client can connect (`curl http://<host>:80/` returns `HTTP/1.0 200 OK`), with no filesystem dependency.

**Architecture:** Start `WebServer.main(new String[0])` as a background thread from the boot Init class `AI.start()` (which runs in the Init domain created by the C kernel at `src/Interface/zero.c:374`). `WebServer` already blocks on `waitUntilPortalAvailable("NET")` in its constructor, so it waits for NET then binds port 80. Each client connection gets its own worker domain via the existing `startWorkerDomain` path.

**Tech Stack:** Java (JCore OS), `test/NET` (WebServer, TCPWebWorker), `test/APP` (AI boot class), `DomainStarter.createDomain` for per-connection worker domains.

## Global Constraints

- No filesystem (`-fs` not used); WebServer serves hardcoded pages only.
- Default port 80 (`WebServer` constructor default).
- `WebServer` constructor blocks on `NET` portal via `waitUntilPortalAvailable` — no boot-ordering change needed.
- Do not modify core OS C kernel or `zero.c`.
- Follow existing patterns: `AI.start()` already launches `USBMassStorageTest.main(null)` in a `Thread` (line 119-124).
- Frequent commits; each task is independently testable.

---

### Task 1: Make WebServer accept loop resilient

**Files:**
- Modify: `test/NET/src/test/net/WebServer.java:101-146`

**Interfaces:**
- Consumes: `java.io.IOException` from `ssock.accept()`; exceptions from `TCPWebWorker.processRequest` and `DomainStarter.createDomain`.
- Produces: a server that stays alive after a bad client connection; worker exceptions are logged instead of swallowed.

- [ ] **Step 1: Write the failing test**

There is no unit-test harness for the kernel Java test suite; verification is by boot + `curl`. Proceed directly to implementation.

- [ ] **Step 2: Replace `break` with log-and-continue in the accept loop**

Current code (lines 101-103):
```java
            } catch (IOException e){                
                break;
            }
```
Replace with:
```java
            } catch (IOException e){
                Debug.out.println("WebServer: accept() failed: " + e.getMessage() + "; continuing");
            }
```

- [ ] **Step 3: Log swallowed worker-thread exceptions**

Current code (lines 122-123):
```java
                } catch(Exception e) {
                    //throw new Error();
                }
```
Replace with:
```java
                } catch(Exception e) {
                    Debug.out.println("WebServer: worker thread failed: " + e.getMessage());
                }
```

- [ ] **Step 4: Log domain-creation failures in startWorkerDomain**

Wrap the `DomainStarter.createDomain` call in `startWorkerDomain` (lines 139-145) so one failed domain creation doesn't crash the accept loop:
```java
    private void startWorkerDomain(final Socket sock){
	String domainName = "Servlet";
	String mainLib = "init.jll";
	String startClass = "test/net/TCPWebWorker";
	String[] argv = new String[0];
	Object[] portals = new Object [] { sock, fs };
	boolean useChunkedGC = false;
	int codesize = 10000;
	try {
	    if (useChunkedGC) {
		int HeapInitialSize = 40000;
		int HeapChunkSize = 10000;
		int StartGCSize = 80000;
		Domain domain = DomainStarter.createDomain(domainName, mainLib, startClass, (String)null, HeapInitialSize, HeapChunkSize, StartGCSize, argv, portals);
	    } else {
		int HeapSize = 400000;
		Domain domain = DomainStarter.createDomain(domainName, mainLib, startClass, HeapSize, codesize, argv, portals);
	    }
	} catch (Exception e) {
	    Debug.out.println("WebServer: could not start worker domain: " + e.getMessage());
	}
    }
```

- [ ] **Step 5: Verify the edit compiles**

Run: `javac` on `test/NET/src/test/net/WebServer.java` against the existing test/NET classpath (or confirm no syntax errors by inspection if javac is unavailable).
Expected: no syntax errors; `Debug` and `Domain` are already imported/available in the file.

- [ ] **Step 6: Commit**

```bash
git add test/NET/src/test/net/WebServer.java
git commit -m "fix: WebServer accept loop stays alive after bad client"
```

---

### Task 2: Start WebServer at boot from the Init class

**Files:**
- Modify: `test/APP/src/AI/AI.java:114-140` (`start()` method)

**Interfaces:**
- Consumes: `test.net.WebServer` (already imported at line 28).
- Produces: a background thread in the boot Init domain that runs `WebServer.main(new String[0])` — binds port 80, serves hardcoded pages, no filesystem.

- [ ] **Step 1: Add WebServer boot thread to `AI.start()`**

After the existing `USBMassStorageTest` thread block (lines 119-124), insert:
```java
        // Start the HTTP server on port 80 (no filesystem). Blocks on the NET
        // portal until the network stack is up, then accepts connections.
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    test.net.WebServer.main(new String[0]);
                } catch (Exception e) {
                    Debug.out.println("WebServer: failed to start: " + e.getMessage());
                }
            }
        }, "WebServer").start();
```
`test.net.WebServer` is already imported (line 28), so `WebServer.main(new String[0])` can be used directly.

- [ ] **Step 2: Confirm WebServer import is present**

Verify line 28 `import test.net.WebServer;` exists (it does). No change needed.

- [ ] **Step 3: Verify the edit**

Run: inspect `test/APP/src/AI/AI.java` around `start()` to confirm the new thread block is placed after the USBMassStorageTest thread and before `System.out.println("AI running...");`.
Expected: WebServer starts in background; `AI.start()` continues to "AI running...".

- [ ] **Step 4: Commit**

```bash
git add test/APP/src/AI/AI.java
git commit -m "feat: start HTTP server on port 80 at boot from AI Init domain"
```

---

### Task 3: Build, boot, and verify port 80 responds

**Files:**
- Test: manual boot + `curl` (no source change)

**Interfaces:**
- Consumes: none.
- Produces: confirmed `HTTP/1.0 200 OK` response from `http://<host>:80/`.

- [ ] **Step 1: Build the OS image**

Run: `make` (per `run.sh` line 3 builds `jxcore`; `run.sh` then zips JLLs and builds `MyOS.iso`).
Expected: `MyOS.iso` produced without errors.

- [ ] **Step 2: Boot and confirm NET is up**

Run: `qemu-system-x86_64 -m 1024 -drive file=disk.img,format=raw -cdrom MyOS.iso -boot d -serial stdio` (per `run.sh` line 25).
Expected: serial output shows NET initialization and an IP address assigned; "AI running..." appears.

- [ ] **Step 3: Connect to port 80**

From a client on the same network: `curl -v http://<host>:80/`.
Expected: `HTTP/1.0 200 OK` followed by the hardcoded index HTML body ("Herzlich willkommen auf der JX-FILE-Testseite").

- [ ] **Step 4: Confirm port stays alive after multiple connections**

Run: `curl http://<host>:80/` two or three times in sequence.
Expected: each returns `HTTP/1.0 200 OK`; the server does not exit after the first connection.

- [ ] **Step 5: Commit test evidence**

```bash
git commit --allow-empty -m "test: verified HTTP server responds on port 80 after boot"
```

---

## Spec Coverage Check

| Spec requirement | Task |
|---|---|
| Boot-time domain starts WebServer on port 80 | Task 2 |
| No filesystem dependency | Task 2 (no `-fs` arg) |
| `accept()` loop doesn't die on one bad client | Task 1 (log + continue) |
| Worker exceptions logged, not swallowed | Task 1 (Steps 3-4) |
| `curl http://<host>:80/` returns 200 | Task 3 |

## Placeholder Scan

No TBD/TODO/placeholder remains. All steps contain exact code, file paths, commands, and expected output. The only judgment calls (build boot via `make`, boot via QEMU per `run.sh`) are documented in Task 3 with the exact commands from `run.sh`.
