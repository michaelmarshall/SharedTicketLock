// SharedTicketLock.h
#pragma once
#include <atomic>
#include <thread>

class SharedTicketLock {
private:
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
