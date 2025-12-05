# FileSync
Distributed File Synchronization System

Goal: Sync files across multiple nodes with versioning, conflict resolution, and fault tolerance.

Architecture Overview

Client-Server Model:

Server Nodes: Store file replicas, manage metadata, handle sync requests.

Clients: Upload/download files.

Communication: gRPC or ZeroMQ (C++).

Persistence: Local file system + JSON/SQLite metadata.

Features:

File versioning and conflict resolution.

Automatic re-sync if server/client restarts.

Heartbeat/Node health check for fault tolerance.
