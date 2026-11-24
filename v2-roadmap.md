# Java Operating System v2.0 — Phased Roadmap and Non-Functional Requirements

**Author:** Xuyi Wang
**Date:** 2026-08-07
**Status:** Draft for review
**Companion to:** Software Design Document — Java Operating System v2.0

---

## 1. Assumptions and scope decisions

This roadmap makes the following decisions explicit, because the SDD leaves them open. Each is a decision to confirm or overturn, not a fact.

| # | Decision | Rationale |
|---|---|---|
| A1 | **Primary target is embedded / appliance-class systems**, not desktop | Bounded driver surface, bounded ecosystem expectations, and a real buyer for a minimal trusted computing base. Desktop is unwinnable against Windows/macOS/Linux on driver support and application gravity. |
| A2 | **Architecture: single-address-space, type-safe (Design A)** | This is the only variant that a hosted JVM on Linux cannot already deliver. A C microkernel hosting a JVM has no thesis. |
| A3 | **Server is a Phase 4 target; desktop is explicitly a non-goal for v2.0** | Server is reachable once the driver and networking stack mature. Desktop is deferred indefinitely. |
| A4 | **x86-64 only through Phase 3**; ARM64 in Phase 4; RISC-V post-v2.0 | Four backends in parallel is the single most common way this class of project stalls. |
| A5 | **Port an existing file system rather than write one** | Writing a new file system is multi-year work with catastrophic failure modes. See Phase 2. |
| A6 | **No hypervisor in v2.0** | Type-1 virtualization is a project of comparable scale to the OS. Moved to v3.0. |
| A7 | **Hybrid AOT + JIT**, not AOT-only | AOT-only breaks reflection-dependent libraries and loses profile-guided optimization on long-running workloads. |

### Non-goals for v2.0

- Desktop environment, compositor, or GPU acceleration
- Running Windows or Linux guests
- POSIX certification
- RISC-V support
- A bespoke on-disk file system format
- Running unmodified enterprise Java frameworks (Spring, JEE) — see R4

---

## 2. Non-functional requirements

All targets are measured against a **Linux baseline** on identical hardware: reference platform is an x86-64 machine, 4 cores, 8 GB RAM, NVMe SSD, running a current mainline Linux LTS kernel with an equivalent workload. Every requirement below is a gate: a phase does not exit until its NFRs are met and reproducibly measured in CI.

### 2.1 Performance

| ID | Requirement | Target | Baseline | Measurement method |
|---|---|---|---|---|
| P1 | Kernel boot to first user domain | ≤ 250 ms | Linux ~800 ms (systemd, minimal) | Hardware timestamp counter from firmware handoff to first domain entry point |
| P2 | Cold boot to network-reachable service | ≤ 1.5 s | Linux ~4 s | Power-on to first successful TCP accept |
| P3 | Null IPC round-trip, same core | ≤ 300 ns | Linux `pipe` ~2–5 µs; seL4 ~0.5 µs | 10M-iteration ping-pong, median and p99 |
| P4 | Null IPC round-trip, cross core | ≤ 1.5 µs | Linux ~8 µs | As above, pinned to distinct cores |
| P5 | Domain (process) creation | ≤ 200 µs | Linux `fork`+`exec` ~1 ms | 100k creations, median and p99 |
| P6 | Sequential file read throughput | ≥ 85% of Linux | 1.0× | `fio`-equivalent, 1 GiB, 1 MiB blocks, O_DIRECT semantics |
| P7 | Random 4K read IOPS | ≥ 75% of Linux | 1.0× | `fio`-equivalent, queue depth 32 |
| P8 | TCP throughput, 10 GbE | ≥ 80% of Linux | 1.0× | iperf-equivalent, single stream and 8 streams |
| P9 | Steady-state throughput, long-running Java service | ≥ 95% of OpenJDK on Linux | 1.0× | Renaissance / SPECjvm-class benchmark suite, 30-minute run after warmup |
| P10 | Java application startup (AOT path) | ≤ 30 ms to first request served | OpenJDK JIT ~800 ms; GraalVM native ~40 ms | Hello-world HTTP service |

### 2.2 Latency and real-time behaviour

Garbage collection is the defining risk for this design. These are hard gates.

| ID | Requirement | Target | Measurement method |
|---|---|---|---|
| L1 | GC pause, privileged/kernel domains | **0 ns — no GC permitted** | Static verification: kernel-domain code passes a no-allocation checker on all paths reachable from interrupt and syscall entry |
| L2 | GC pause, driver domains | ≤ 50 µs p99.9 | Per-domain heap, region-allocated; instrumented pause log over 24 h |
| L3 | GC pause, application domains | ≤ 2 ms p99, ≤ 10 ms p99.99 | 24 h soak under 80% heap pressure |
| L4 | Interrupt-to-handler latency | ≤ 5 µs p99.9 | Hardware loopback timer, 24 h, including under full GC load in an unrelated domain |
| L5 | Scheduler jitter, real-time class | ≤ 100 µs p99.9 | Cyclictest-equivalent, 24 h under load |
| L6 | GC in one domain must not delay another domain | ≤ 10 µs added latency p99 | Cross-domain interference test: application domain in full GC, driver domain measured |

L6 is the requirement that per-domain heaps exist to satisfy. If it cannot be met, the single-address-space design is not viable for the embedded target and A2 must be revisited.

### 2.3 Security

| ID | Requirement | Target | Verification |
|---|---|---|---|
| S1 | Trusted computing base size | ≤ 20,000 lines of code | Automated LOC count per release, published |
| S2 | Non-TCB code is memory-safe | 100% | No unsafe/FFM/JNI outside TCB; build fails on violation |
| S3 | Bytecode verifier correctness | Formally specified; differential-tested against reference JVM | ≥ 10M generated programs per release, zero divergences |
| S4 | JIT/AOT output preserves verifier guarantees | Translation validation on every compiled method | Per-method proof obligation checked in CI on the full benchmark corpus |
| S5 | Capability model | No ambient authority; every resource access requires an explicit capability | Audit: zero global namespaces reachable without a capability |
| S6 | Secure boot chain | Firmware → kernel → TCB measured and signed | TPM 2.0 attestation, reproducible builds |
| S7 | Driver compromise containment | A malicious driver domain cannot read another domain's heap | Red-team test suite, ≥ 40 documented attack scenarios |
| S8 | Critical vulnerability remediation | Patch available ≤ 7 days | Tracked per CVE |

### 2.4 Reliability

| ID | Requirement | Target | Measurement |
|---|---|---|---|
| R1 | Mean time between kernel panics | ≥ 90 days under continuous load | Fleet soak, ≥ 20 machines |
| R2 | Zero data loss on unexpected power loss | 100% of acknowledged writes durable | ≥ 5,000 automated power-cut cycles |
| R3 | File system recovery after unclean shutdown | ≤ 3 s, no manual intervention | Same power-cut harness |
| R4 | Java compatibility | 100% of a defined 200-library corpus runs unmodified | Curated corpus of embedded-relevant libraries; enterprise frameworks explicitly excluded |
| R5 | Driver domain crash does not take down the system | Restart within 500 ms, no kernel involvement | Fault-injection suite |
| R6 | JCK-equivalent conformance for the supported Java subset | ≥ 99% pass | Per-release run |

### 2.5 Resource footprint

| ID | Requirement | Target | Baseline |
|---|---|---|---|
| F1 | Kernel + TCB memory resident | ≤ 8 MB | Linux minimal ~40 MB |
| F2 | Minimum bootable system image | ≤ 64 MB | Alpine Linux ~130 MB |
| F3 | Idle CPU usage, no workload | ≤ 0.1% | Linux ~0.2% |
| F4 | Per-domain overhead | ≤ 256 KB | Linux process ~1–2 MB RSS |

### 2.6 Developer experience

| ID | Requirement | Target |
|---|---|---|
| D1 | Clean full-system build from source | ≤ 15 min on 8-core developer machine |
| D2 | Edit-to-running-on-QEMU cycle | ≤ 45 s |
| D3 | Source-level debugging of kernel-domain Java code | Supported via standard JDWP-compatible tooling |
| D4 | Public API documentation coverage | 100% of exported interfaces |
| D5 | Reproducible builds | Bit-identical output across two independent machines |

---

## 3. Phased roadmap

Eight phases across roughly 42 months to v2.0 general availability. Each phase has an exit gate; the project does not advance until the gate is met. Phase 0 exists specifically to kill the project cheaply if the thesis does not hold.

### Phase 0 — Thesis validation (Q3 2026 – Q4 2026, 4 months)

**Purpose:** prove the single-address-space performance claim before committing to a multi-year build.

**Deliverables**
- Prototype single-address-space IPC on bare metal x86-64, no file system, no drivers beyond serial and timer
- Per-domain heap prototype with a no-allocation verifier for privileged paths
- Measured comparison against Linux pipes, Linux shared memory, and published seL4 numbers
- Written "Why not a JVM on Linux?" thesis document, ≤ 3 pages, with data
- Decision record confirming or overturning A1, A2, A4

**Exit gate**
- P3 met (≤ 300 ns null IPC round-trip)
- L1 demonstrated on the prototype's syscall path
- L6 demonstrated with two domains
- Thesis document reviewed and accepted

**Kill criterion:** if IPC round-trip exceeds 1 µs, the design offers nothing over a hosted JVM. Stop the project or revert to Design B and rescope entirely.

---

### Phase 1 — Kernel and runtime core (Q1 2027 – Q4 2027, 12 months)

**Deliverables**
- Minimal TCB: domain management, scheduling, memory management, IPC, capability enforcement
- Bytecode verifier with a written formal specification
- Bootstrap chain documented and reproducible: cross-compiled seed compiler → self-hosted compiler
- Baseline JIT compiler, x86-64 only
- Capability model specification and implementation
- Boot from UEFI on the reference platform
- Serial console, timer, interrupt controller, basic MMU setup

**Exit gate**
- P1, P4, P5 met
- L1, L4, L5 met
- S1 (≤ 20k LOC TCB), S2, S3, S5 met
- D1, D2, D5 met
- Boots reliably on reference hardware and QEMU

**Primary risks:** bootstrap complexity; TCB LOC budget overrun. Mitigation: LOC budget tracked weekly from day one, with a hard freeze at 18k lines forcing feature removal.

---

### Phase 2 — Storage and device model (Q1 2028 – Q3 2028, 9 months)

**Deliverables**
- Driver domain model: memory-safe DMA and MMIO abstractions, interrupt delivery to Java domains, IOMMU-enforced isolation
- Drivers: NVMe, AHCI, virtio-blk
- **File system decision executed** — port an existing, proven implementation rather than write one. Evaluate in order: (1) a clean-room Java implementation of the ext4 on-disk format, read-write; (2) a copy-on-write design reusing an existing verified format. Write a new format only if both are rejected with documented reasons.
- Block layer, page cache, VFS-equivalent abstraction
- Crash-consistency test harness with automated power-cut rig

**Exit gate**
- P6, P7 met
- R2 met over 5,000 power-cut cycles
- R3, R5 met
- L2 met
- S7 met with ≥ 40 attack scenarios

**Primary risks:** driver work is historically where this class of project dies. Mitigation: hard scope limit to three storage controllers and two NICs for v2.0; anything else is out of scope. Budget 40% of total engineering effort across Phases 2 and 3 to device support.

---

### Phase 3 — Networking, AOT pipeline, and Java compatibility (Q4 2028 – Q3 2029, 12 months)

**Deliverables**
- Network stack: Ethernet, IPv4/IPv6, TCP, UDP, TLS
- Drivers: virtio-net plus one physical 10 GbE NIC
- AOT compilation pipeline with a defined compilation site (install time), signed output, and cache invalidation policy
- Tiered execution: AOT for boot path and startup, JIT with profile-guided optimization for long-running code
- Reflection and dynamic class loading strategy — this is the compatibility crux. AOT closes the world, and [GraalVM Native Image requires all reachable methods to be known at build time](https://www.graalvm.org/22.0/reference-manual/native-image/Limitations/) with [reflection targets registered ahead of time](https://www.graalvm.org/jdk21/reference-manual/native-image/dynamic-features/Reflection/). Deliver a documented hybrid: AOT-compiled closed world plus a JIT fallback path for dynamically loaded code, with the security implications of that fallback specified.
- 200-library compatibility corpus defined, automated, and passing
- SSH server and CLI environment
- Secure boot with TPM attestation

**Exit gate**
- P2, P8, P9, P10 met
- L3 met over 24 h soak
- R4 met (200/200 libraries)
- R6 met (≥ 99% conformance on the supported subset)
- S4, S6, S8 met
- F1–F4 met

**Primary risks:** the JIT fallback path is a hole in both the AOT performance story and the closed-world security story. If it cannot be made safe, R4 must be renegotiated downward and the compatibility claim in the SDD softened.

---

### Phase 4 — Hardening, ARM64, and server profile (Q4 2029 – Q2 2030, 9 months)

**Deliverables**
- ARM64 backend, second reference platform
- 20-machine soak fleet running continuously
- External security audit of the TCB and verifier
- Red-team engagement, findings remediated
- Server profile: multi-tenant domain isolation, resource quotas, observability and audit logging
- Formal verification of the capability enforcement core (stretch goal — scope to the enforcement core only, not the whole TCB)

**Exit gate**
- R1 met (≥ 90 days MTBF across the fleet)
- All Phase 1–3 NFRs re-verified on ARM64
- External audit findings closed
- D3, D4 met

---

### Phase 5 — v2.0 general availability (Q3 2030)

**Deliverables**
- Full NFR compliance report, published, with reproducible benchmark harness
- Complete documentation set: architecture, security model, driver-authoring guide, porting guide
- Reference embedded appliance image
- Long-term support and CVE process operational

**Exit gate:** every NFR in Section 2 met and independently reproducible.

---

### Deferred to v3.0

Type-1 hypervisor and Windows/Linux guest support; RISC-V; graphical desktop, compositor, and GPU acceleration; enterprise framework compatibility.

---

## 4. Risk register

| ID | Risk | Impact | Likelihood | Mitigation | Owner phase |
|---|---|---|---|---|---|
| RK1 | IPC performance does not beat a hosted JVM | Fatal — project has no thesis | Medium | Phase 0 kill gate before major investment | 0 |
| RK2 | Driver support consumes the schedule | Fatal — the historical cause of death for [JavaOS](https://en.wikipedia.org/wiki/JavaOS) and [JNode](https://www.reddit.com/r/java/comments/1g1ybs4/is_there_any_active_opensource_java_os/) | **High** | Hard scope cap: 3 storage controllers, 2 NICs, 1 reference board per architecture | 2, 3 |
| RK3 | TCB exceeds 20k LOC | Undermines the entire security thesis | High | Weekly LOC tracking, hard freeze at 18k forcing cuts | 1 |
| RK4 | GC pauses violate L2/L4/L6 | Design non-viable for embedded | Medium | Per-domain heaps and no-allocation verification designed in Phase 0, not retrofitted | 0, 1 |
| RK5 | AOT closed-world breaks ecosystem compatibility | Erodes the core "large ecosystem" advantage | High | Narrow the corpus to embedded-relevant libraries; exclude enterprise frameworks explicitly in the SDD | 3 |
| RK6 | Verifier or JIT bug becomes total system compromise | Catastrophic — no hardware fallback in a single address space | Medium | S3 differential testing, S4 translation validation, external audit, formal verification of the enforcement core | 1, 4 |
| RK7 | No adoption despite technical success | Project succeeds and still fails, as [JX](https://en.wikipedia.org/wiki/JX_(operating_system)) did | High | Identify a named design partner with a concrete appliance use case before Phase 2 exits | 2 |
| RK8 | Bootstrap chain proves intractable | Schedule slip of 6+ months | Low | Cross-compile the seed from a hosted JVM; self-hosting is a Phase 1 goal, not a Phase 0 prerequisite | 1 |

RK7 deserves emphasis. Prior efforts failed less on engineering than on the absence of a user who needed them. A named design partner by end of Phase 2 should be treated as a gate condition, not a wish.

## 5. Measurement infrastructure

None of the above is meaningful without the harness. Build it in Phase 0, not later.

- **Continuous benchmarking:** every NFR runs on every merge against the reference hardware; regressions above 3% block the merge
- **Public dashboard:** every metric in Section 2 plotted over time against its Linux baseline
- **Automated power-cut rig:** relay-controlled PSU, required from Phase 2
- **Soak fleet:** 20 machines from Phase 4, continuous, with automatic panic capture
- **Reproducible baselines:** the Linux comparison runs on the same hardware in the same CI cycle, never quoted from memory or from published third-party numbers
