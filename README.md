# Architecture & Implementation Details
### 1. The Producer-Consumer Pattern
The application splits the workload into two distinct roles to maximize CPU utilization:

Producer: Reads network packets from the input source and pushes them into a shared Ring Buffer.

Consumers: Multiple worker threads pop packets from the Ring Buffer and analyze them against a set
of firewall rules to decide whether to ACCEPT or DROP the packet.

### 2. Ring Buffer (Shared Memory)
To allow safe communication between threads, a Ring Buffer (Circular Buffer) is implemented. Access
to the buffer is synchronized using mutexes and semaphores/condition variables to prevent race
conditions and ensure data integrity.

### 3. Packet Filtering
The firewall inspects packet headers (IP, TCP/UDP ports) and matches them against a rule database.
The processing logic is designed to be thread-safe, allowing multiple consumers to filter different packets simultaneously.
