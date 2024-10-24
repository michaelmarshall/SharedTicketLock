# Shared Ticket Lock

A very simple, shared ticket lock whose goal is to be fair for both readers and writers in the order they arrive.

- Everyone gets in the same line
- Line-contiguous reader groups get concurrent access
- A writer waits for all readers ahead of it and then gets access
- After a writer unlocks, the next contiguous reader group gets access

```cpp
// SharedTicketLock.h
#pragma once
#include <atomic>
#include <thread>

class SharedTicketLock {
    std::atomic<uint16_t> next_ticket{0};
    std::atomic<uint16_t> now_serving{0};
    std::atomic<uint16_t> served{0};

public:
    void writer_lock(){
        uint16_t my_ticket = next_ticket.fetch_add(2);
        while (now_serving.load() != my_ticket || now_serving.load() != served.load()){}
    }

    void reader_lock(){
        uint16_t my_ticket = next_ticket.fetch_add(1);
        while (now_serving.load() != my_ticket){}
        now_serving.fetch_add(1);
    }

    void writer_unlock(){
        now_serving.fetch_add(2);
        served.fetch_add(2);
    }

    void reader_unlock(){
        served.fetch_add(1);
    }
};
```

- We add an additional atomic counter to the traditional ticket lock: `served` to track the number of readers in the critical section.
- Writers take out two tickets: `next_ticket.fetch_add(2)`, while readers take out just one.
- Readers enter when `now_serving` equals `my_ticket`.
- Writers enter when `now_serving` equals `my_ticket` as well as when `now_serving` equals `served`, indicating all previous entrants have left the critical section.
- When a reader locks, it immediately increments `now_serving`, which in this implementation is used to determine the in-flight readers and signals to the next entrant that if they are a reader, they may enter. If the next entrant is a writer, it must wait for `now_serving` and `served` to be equal.
- When a writer locks, it does not increment `now_serving`, creating a two-ticket gap between the entrant after the writer and causing everyone to wait.
- When a reader unlocks, it increments `served`, which is used by writers to know when all readers before its ticket count have left.
- When a writer unlocks, it double increments both `next_ticket` and `now_serving`, signaling to the next entrant that a writer has left the critical section and they may now enter.
- If the next entrants are readers, they will all enter as each one increments `now_serving` and checks for a 1 ticket gap.
- Ticket overflow is handled implicity as only equality checks are made.
