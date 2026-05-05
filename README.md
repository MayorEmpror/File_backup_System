# Distributed File Backup Manager

**Project ID:** 24K0694, 24K0994, 24K0973  
**Team Members:** Hashir Ali, Izaan Khan, Talha Mirza  

---

## 📌 Project Title
**Distributed File Backup Manager**

---

## 📖 Overview
The **Distributed File Backup Manager** is a multi-threaded system designed to simulate enterprise-level backup solutions. It allows multiple clients to request backups concurrently while dedicated worker threads process these backups efficiently.

This project focuses on **concurrency control, synchronization, and I/O management**, ensuring safe and efficient handling of shared resources.

### 🔑 Key Highlights
- Client-server style architecture for backup requests  
- Concurrent processing of file chunks using worker threads  
- Mutex-based protection for shared metadata and indexes  
- Semaphore-controlled disk write operations  
- Simulation of enterprise features:
  - Incremental backups  
  - Failure recovery  
  - Compression  
  - Logging and progress tracking  

---

## 🎯 Objectives
- Build a multi-client backup system with concurrent request handling  
- Ensure thread-safe operations using **mutexes**  
- Control disk access using **semaphores**  
- Implement advanced backup features:
  - Incremental backups (store only modified data)
  - Failure recovery mechanisms
  - Compression simulation
  - Logging and progress tracking  
- Develop a **GUI (Qt)** for monitoring backup progress  
- Prepare for viva on concurrency and synchronization concepts  

---
<p align="center">
  <img src="https://github.com/user-attachments/assets/699b4e03-11dd-4691-b0e1-dc2aca21d9ac" width="600"/>
</p>

## 🏗 System Architecture

### 🧑 Clients
- Generate backup requests
- Run as separate threads

### ⚙️ Workers
- Process backup requests
- Split files into chunks
- Write data to storage

### 💾 Shared Storage
- Simulated disk storage for backups

### 🔒 Synchronization
- **Mutexes:** Protect metadata and backup indexes  
- **Semaphores:** Limit concurrent disk write operations  

---

## 🔄 Workflow
1. Clients generate file backup requests  
2. Requests are placed into a shared queue  
3. Worker threads process requests:
   - Split files into chunks  
   - Acquire semaphore before writing to disk  
   - Update metadata safely using mutex  
4. Optional enhancements:
   - Compression of file chunks  
   - Incremental backup handling  
   - Failure detection and recovery  
   - Logging of backup operations  

---

## 🛠 Tools & Technologies
- **Language:** C++ (Multithreading supported)  
- **GUI Framework:** Qt (C++)  
- **Data Structures:**
  - Queues (request handling)
  - Maps/Dictionaries (metadata storage)

---

## 📦 Deliverables
- Multi-threaded backup system  
- Mutex-protected metadata handling  
- Semaphore-controlled disk access  
- Optional features:
  - Incremental backups  
  - Compression simulation  
  - Failure recovery system  
- Qt-based GUI dashboard  
- Logging system (timestamps, status, errors)  
- Final demonstration and viva preparation  

---

## 🎓 Expected Outcomes
- Strong understanding of multithreading and concurrency  
- Practical experience with I/O-heavy systems  
- Implementation of synchronization primitives in real systems  
- Functional distributed-style backup manager  
- Interactive GUI for monitoring system activity  

---

## 🚀 Future Improvements
- Real distributed deployment over network nodes  
- Cloud storage integration  
- Real compression algorithms  
- Encryption for secure backups  

---

## 📄 License
This project is developed for academic purposes.
