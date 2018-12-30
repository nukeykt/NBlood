#pragma once

struct HEAPNODE
{
    HEAPNODE *prev;
    HEAPNODE *next;
    int size;
    bool isFree;
    HEAPNODE *freePrev;
    HEAPNODE *freeNext;
};

class QHeap
{
public:
    QHeap(long heapSize);
    ~QHeap(void);

    void Check(void);
    void Debug(void);
    void *Alloc(long);
    long Free(void *p);

    void *heapPtr;
    HEAPNODE heap;
    HEAPNODE freeHeap;
    int size;
};