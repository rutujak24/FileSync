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


###10-Day Plan

Day 1 – Architecture & Model Selection

Decide LLM (open-source: LLaMA, MPT) or API (OpenAI).

Design pipeline: Ingest → Preprocess → Summarize → Query → Respond.

Day 2 – Document Preprocessing

Extract text from PDF/DOCX/TXT.

Implement chunking for long docs.

Basic cleaning (remove newlines, special chars).

Day 3 – Summarization Module

Integrate LLM for summarization.

Test paragraph-level and full-doc summarization.

Day 4 – QA Module

Implement embedding-based retrieval (FAISS or LangChain).

Query document chunks and get LLM answer.

Day 5 – Fine-Tuning / Prompt Engineering

Test different prompts for better summary/QA quality.

Optional: small fine-tune on domain-specific docs.

Day 6 – API & Backend Setup

Build REST API (FastAPI/Flask) for:

Upload document

Get summary

Ask question

Day 7 – Frontend

Build simple React UI:

Upload file, display summary, input question → display answer.

Day 8 – Hosting & Deployment

Dockerize backend + frontend.

Deploy on AWS/GCP/Render.

Day 9 – Testing & Optimization

Test with multiple docs.

Optimize chunking, response speed, memory usage.

Day 10 – Documentation & GitHub

Write README: setup, usage, architecture, sample docs.

Add sample screenshots and hosted link demo.
