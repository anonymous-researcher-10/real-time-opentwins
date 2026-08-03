# Synchronization and Temporal Consistency in Distributed Digital Twins: A Real-Time Architectural Approach

A real-time digital twin platform built around a low-latency C **engine** for ingesting, processing, and storing telemetry from edge devices, plus a small C++ HTTP **API** for auxiliary configuration/admin tasks.

## Architecture

```
edge devices ──▶ ZeroMQ/MessagePack──▶ event_reciever ──▶ event_queue ──▶ event_processor
                                                                              │
                                                                              ▼
                                                                     event_dispatcher
                                                                       │          │
                                                          rule_engine ◀┘          │
                                                                                  ▼
                                                                          twins_memory
                                                                        (lock-free store)
                                                                                  │
                                              ┌───────────────────────────────────┤
                                              ▼                                   ▼
                                       event_sender                          api_server
                                (ZMQ PUSH to downstream                (ZMQ REP RPC on :5556,
                                  Raspberry Pi nodes)                    query/delete twins)
```

### `engine/` (C)

The real-time core. Each stage runs on its own pthread, pinned to a CPU core with real-time scheduling priority (`core_binding.c`) for predictable low-latency processing:

- **event_reciever** — binds a ZeroMQ `PULL` socket (`tcp://*:5555`), decodes incoming MessagePack payloads (via the vendored `cwpack` library) into `event_t` structs, and pushes them onto a ring-buffer queue.
- **event_queue** — fixed-size, thread-safe ring buffer used between stages.
- **event_processor** — pops events off the queue and hands them to the dispatcher.
- **event_dispatcher** — for update events, writes new values into `twins_memory`, then consults the **rule_engine** per variable to decide the action (ignore, notify upwards, halt, set alarm, call a user service). For error events, it looks up a default per-error-type action table (e.g. `SENSOR_FAILURE` → halt the engine).
- **rule_engine** — static rule tables mapping twins/variables and error types to actions.
- **twins_memory** — a lock-free (atomics-based) in-memory store for up to `MAX_TWINS` twins × `MAX_VARS_PER_TWINS` variables, with per-variable status and last-update timestamps.
- **event_sender** — batches events flagged for "notify upwards" and forwards them via ZMQ `PUSH` to a configured list of downstream nodes.
- **api_server** — a ZeroMQ `REP` socket (`tcp://*:5556`) exposing a minimal MessagePack-framed RPC for querying/removing twins from `twins_memory` (get all, get by id, delete). This is separate from the C++ HTTP API below.

Networking and encoding are handled with **ZeroMQ** and **MessagePack** (vendored `cwpack`); there is no external message broker.

### `api/` (C++)

A standalone HTTP service built on the header-only [Crow](https://github.com/CrowCpp/Crow) framework (vendored under `api/include/external/`) over standalone Asio, listening on port `8080`. Routes are organized as controller/service pairs:

- **ChronyController** (`PATCH /configure`, `POST /restart`) — edits a local `chrony` NTP configuration file and restarts the daemon, for time-syncing edge nodes.
- **OtroController** (`GET /otro`) — placeholder/example route.




### `engine/utils_testing/` and `engine/validation/`

Manual test clients (C and Python, ZMQ send/receive, RPC client) and scripts for measuring power/resource consumption, plus latency-benchmark artifacts (distribution plots) from prior test runs.


### `old-ot-lw-testing/`

Legacy Python prototype (`paho-mqtt` client) used before the project moved from MQTT to ZeroMQ/MessagePack. Kept for reference; superseded by `engine/`.




## Building

### Engine

There is no single build script; each test target under `engine/tests/` compiles the relevant engine sources directly with `gcc`, e.g.:

```bash
cd engine/tests
./compile_full_test_v2.sh   # full pipeline: queue + dispatcher + processor + receiver + sender + rule_engine + core_binding
```

Other targets exercise individual components: `compile_queue_test.sh`, `compile_memory_test.sh`, `compile_processor_test.sh`, `compile_reciever_dispatcher_test.sh` / `_3_queues_test.sh`, `compile_rpc_server_test.sh`.

**Requirements:** `libzmq` (ZeroMQ), POSIX threads (`pthread`), `librt` (some tests).


Engine networking, buffer sizes, and downstream node addresses are set at compile time in [engine/include/config.h](engine/include/config.h) (`ZMQ_PORT`, `QUEUE_SIZE`, `MAX_TWINS`, `RBPI3_url`…`RBPI7_url`, etc.). Update these before building for your deployment's network topology.
