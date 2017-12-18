//============================================================================
// Name        : bus2.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#define RING_SIZE 50
using namespace std;

class lockless_ring_buffer
{
private:

    int size=RING_SIZE;

public:
    int buffer[RING_SIZE];

    std::atomic<int> write;
    std::atomic<int> read;
    int increment(int n){
        return (n+1)%size;
        }
    lockless_ring_buffer(){
        write.store(0);
        read.store(0);
    }
};
class Consumer{
    lockless_ring_buffer * ring;
    public:
    Consumer(lockless_ring_buffer*ringBuffer):ring(ringBuffer){}
 bool canRead(int* pval);
 int Read();
};
class Producer{
    lockless_ring_buffer * ring;
    public:
    Producer(lockless_ring_buffer*ringBuffer):ring(ringBuffer){}
 bool canWrte(int i);
 void Write(int i);
};

void Producer::Write(int i){
     while( ! canWrte(i) );
}
bool Producer::canWrte(int i){
    const auto current_tail = ring->write.load();
    const auto next_tail = ring->increment(current_tail);
                if (next_tail != ring->read.load())
                {
                    ring->buffer[current_tail] = i;
                    ring->write.store(next_tail);
                    return true;
                }

                return false;
}
int Consumer::Read(){
int num;
while(! canRead(&num));
return num;
}
bool Consumer::canRead(int* pval){
    auto currentHead=ring->read.load();
    if (currentHead==ring->write.load()){
        //индекс чтения и записи на одной строке
        return false;
    }
    //cut
    *pval=ring->buffer[currentHead];
    ring->read.store((ring->increment(currentHead)));
    //hear
    return true;
}
int main(){
    lockless_ring_buffer queue;
    std::thread write_thread([&](){
        Producer producer(&queue);
        for(int i=0;i<1000000; i++)
            {
             producer.Write(i);
            }
    });
    std::thread read_thread([&] (){
        Consumer consumer(&queue);
        for(int i=0;i<1000000; i++){
            consumer.Read();
        }
    });
    write_thread.join();
    read_thread.join();
    return 0;
}
