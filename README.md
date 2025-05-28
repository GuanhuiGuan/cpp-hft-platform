# cpp-hft-platform

## Notes
- use allocator to avoid dynamic allocation
- lock-free queues for data transfer
- can use std::array to implement hashmap (number as key)
- graceful shutdown: std::signal(SIGINT, signalHandler)