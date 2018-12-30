#pragma once
#define kPQueueSize 1024

class PriorityQueue
{
public:
    PriorityQueue();
    void Upheap(void);
    void Downheap(unsigned int);
    void Delete(unsigned int);
    void Insert(unsigned long, unsigned long);
    unsigned long Remove(void);

    struct queueItem
    {
        unsigned long at0; // priority
        unsigned long at4; // data
    } queueItems[kPQueueSize+1];

    int fNodeCount; // at2008
};