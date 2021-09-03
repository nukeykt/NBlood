
#include "compat.h"
#include "hash.h"
#include "baselayer.h"

// vaguely inspired by https://databasearchitects.blogspot.com/2020/01/all-hash-table-sizes-you-will-ever-need.html
// all things considered, I would have just used that implementation if MSVC had any support for __int128
static constexpr uint64_t primes[] = {
    11ull,     13ull,     17ull,     19ull,     37ull,      41ull,      43ull,     59ull,     67ull,     73ull,     79ull,     83ull,     109ull,    113ull,
    131ull,    139ull,    149ull,    157ull,    163ull,     173ull,     191ull,    211ull,    227ull,    241ull,    257ull,    269ull,    283ull,    307ull,
    331ull,    349ull,    373ull,    397ull,    419ull,     439ull,     461ull,    499ull,    523ull,    557ull,    587ull,    617ull,    647ull,    691ull,
    727ull,    769ull,    809ull,    877ull,    941ull,     997ull,     1049ull,   1103ull,   1163ull,   1223ull,   1283ull,   1367ull,   1439ull,   1523ull,
    1609ull,   1693ull,   1783ull,   1871ull,   1987ull,    2087ull,    2203ull,   2333ull,   2459ull,   2591ull,   2719ull,   2857ull,   2999ull,   3163ull,
    3323ull,   3491ull,   3671ull,   3863ull,   4091ull,    4297ull,    4513ull,   4751ull,   4987ull,   5237ull,   5501ull,   5791ull,   6079ull,   6397ull,
    6733ull,   7079ull,   7433ull,   7877ull,   8269ull,    8681ull,    9127ull,   9587ull,   10067ull,  10589ull,  11117ull,  11677ull,  12263ull,  12893ull,
    13537ull,  14221ull,  14947ull,  15727ull,  16519ull,   17351ull,   18217ull,  19139ull,  20101ull,  21107ull,  22189ull,  23297ull,  24473ull,  25717ull,
    27011ull,  28387ull,  29819ull,  31321ull,  32887ull,   34537ull,   36263ull,  38083ull,  39989ull,  41999ull,  44101ull,  46307ull,  48623ull,  51059ull,
    53611ull,  56333ull,  59149ull,  62119ull,  65239ull,   68501ull,   71933ull,  75533ull,  79309ull,  83273ull,  87481ull,  91867ull,  96461ull,  101287ull,
    106357ull, 111697ull, 117281ull, 123191ull, 129379ull,  135851ull,  142657ull, 149791ull, 157279ull, 165161ull, 173431ull, 182101ull, 191227ull, 200789ull,
    210827ull, 221399ull, 232499ull, 244157ull, 256369ull,  269189ull,  282661ull, 296797ull, 311677ull, 327263ull, 343627ull, 360817ull, 378869ull, 397811ull,
    417719ull, 438611ull, 460543ull, 483611ull, 507803ull,  533213ull,  559877ull, 587891ull, 617311ull, 648181ull, 680597ull, 714673ull, 750413ull, 787981ull,
    827389ull, 868771ull, 912211ull, 957821ull, 1005761ull, 1056049ull,
};

static uint64_t hash_findprime(uint64_t desiredSize)
{
    if (desiredSize >= primes[ARRAY_SIZE(primes)-1])
    {
        initprintf("hash_findprime(): size %llu requested, expand prime table\n", desiredSize);
        return primes[ARRAY_SIZE(primes)-1];
    }

    uint64_t lower = 0, upper = ARRAY_SIZE(primes)-1;
    while (lower != upper)
    {
        uint64_t middle = lower + ((upper - lower) >> 1);
        if (primes[middle] < desiredSize)
            lower = middle + 1;
        else if (primes[middle] > desiredSize)
            upper = middle;
        else
            return primes[middle];
    }
    return primes[lower];
}

void hash_init(hashtable_t *t)
{
    hash_free(t);
    t->prime.v = hash_findprime(t->size);
    t->prime.d = libdivide::libdivide_u32_gen(t->prime.v);
    t->items = (hashitem_t **) Xaligned_calloc(16, t->prime.v, sizeof(hashitem_t));
}

void hash_loop(hashtable_t *t, void(*func)(const char *, intptr_t))
{
    if (t->items == nullptr)
        return;

    for (unsigned i=0; i < t->prime.v; i++)
        for (auto item = t->items[i]; item != nullptr; item = item->next)
            func(item->string, item->key);
}

void hash_free(hashtable_t *t)
{
    if (t->items == nullptr)
        return;

    int remaining = t->prime.v - 1;

    do
    {
        auto cur = t->items[remaining];

        while (cur)
        {
            auto tmp = cur;
            cur = cur->next;

            Xfree(tmp->string);
            Xaligned_free(tmp);
        }
    } while (--remaining >= 0);

    ALIGNED_FREE_AND_NULL(t->items);
}

void hash_add(hashtable_t *t, const char *s, intptr_t key, int32_t replace)
{
#ifdef DEBUGGINGAIDS
    Bassert(t->items != nullptr);
#endif
    uint32_t const code = hash_getbucket(t,s);
    auto cur = t->items[code];

    if (!cur)
    {
        cur = (hashitem_t *) Xaligned_alloc(16, sizeof(hashitem_t));
        cur->string = Xstrdup(s);
        cur->key    = key;
        cur->next   = nullptr;

        t->items[code] = cur;
        return;
    }

    hashitem_t *prev = nullptr;

    do
    {
        if (Bstrcmp(s, cur->string) == 0)
        {
            if (replace) cur->key = key;
            return;
        }
        prev = cur;
    } while ((cur = cur->next));

    cur = (hashitem_t *) Xaligned_alloc(16, sizeof(hashitem_t));
    cur->string = Xstrdup(s);
    cur->key    = key;
    cur->next   = nullptr;

    prev->next = cur;
}

// delete at most once
void hash_delete(hashtable_t *t, const char *s)
{
#ifdef DEBUGGINGAIDS
    Bassert(t->items != nullptr);
#endif
    uint32_t const code = hash_getbucket(t,s);
    auto cur = t->items[code];

    if (!cur)
        return;

    hashitem_t *prev = nullptr;

    do
    {
        if (Bstrcmp(s, cur->string) == 0)
        {
            Xfree(cur->string);

            if (!prev)
                t->items[code] = cur->next;
            else
                prev->next = cur->next;

            Xaligned_free(cur);

            break;
        }
        prev = cur;
    } while ((cur = cur->next));
}

intptr_t hash_find(const hashtable_t * const t, char const * const s)
{
#ifdef DEBUGGINGAIDS
    Bassert(t->items != nullptr);
#endif
    auto cur = t->items[hash_getbucket(t,s)];

    if (!cur)
        return -1;

    do
        if (Bstrcmp(s, cur->string) == 0)
            return cur->key;
    while ((cur = cur->next));

    return -1;
}

intptr_t hash_findcase(const hashtable_t * const t, char const * const s)
{
#ifdef DEBUGGINGAIDS
    Bassert(t->items != nullptr);
#endif
    auto cur = t->items[hash_getbucket(t,s)];

    if (!cur)
        return -1;

    do
        if (Bstrcasecmp(s, cur->string) == 0)
            return cur->key;
    while ((cur = cur->next));

    return -1;
}


void inthash_free(inthashtable_t *t) { ALIGNED_FREE_AND_NULL(t->items); }

void inthash_init(inthashtable_t *t)
{
    inthash_free(t);
    t->prime.v = hash_findprime(t->count);
    t->prime.d = libdivide::libdivide_u32_gen(t->prime.v);
    t->items = (inthashitem_t *) Xaligned_calloc(16, t->prime.v, sizeof(inthashitem_t));
}

void inthash_loop(inthashtable_t const *t, void(*func)(intptr_t, intptr_t))
{
    if (t->items == nullptr)
        return;

    for (auto *item = t->items, *const items_end = t->items + t->prime.v; item < items_end; ++item)
        func(item->key, item->value);
}


void inthash_add(inthashtable_t *t, intptr_t key, intptr_t value, int32_t replace)
{
#ifdef DEBUGGINGAIDS
    Bassert(t->items != nullptr);
#endif
    auto seeker = t->items + inthash_getbucket(t,key);

    if (seeker->collision == nullptr)
    {
        seeker->key = key;
        seeker->value = value;
        seeker->collision = seeker;
        return;
    }

    if (seeker->key == key)
    {
        if (replace)
            seeker->value = value;
        return;
    }

    while (seeker != seeker->collision)
    {
        seeker = seeker->collision;

        if (seeker->key == key)
        {
            if (replace)
                seeker->value = value;
            return;
        }
    }

    auto tail = seeker;
    int ofs;

    do
    {
        ofs = (tail - t->items + 1);
        tail = t->items + ofs - libdivide::libdivide_u32_do(ofs, &t->prime.d) * t->prime.v;
    }
    while (tail->collision != nullptr && tail != seeker);

    if (EDUKE32_PREDICT_FALSE(tail == seeker))
        fatal_exit("inthash_add(): table full!\n");

    tail->key = key;
    tail->value = value;
    tail->collision = seeker->collision = tail;
}

// delete at most once
void inthash_delete(inthashtable_t *t, intptr_t key)
{
#ifdef DEBUGGINGAIDS
    Bassert(t->items != nullptr);
#endif
    auto seeker = t->items + inthash_getbucket(t,key);

    if (seeker->collision == nullptr || seeker->key == key)
    {
        seeker->collision = nullptr;
        return;
    }

    while (seeker != seeker->collision)
    {
        auto prev = seeker;
        seeker = seeker->collision;

        if (seeker->key == key)
        {
            prev->collision = seeker == seeker->collision ? prev : seeker->collision;
            seeker->collision = nullptr;
            return;
        }
    }
}

intptr_t inthash_find(inthashtable_t const *t, intptr_t key)
{
#ifdef DEBUGGINGAIDS
    Bassert(t->items != nullptr);
#endif
    auto seeker = t->items + inthash_getbucket(t,key);

    if (seeker->collision == nullptr)
        return -1;

    if (seeker->key == key)
        return seeker->value;

    while (seeker != seeker->collision)
    {
        seeker = seeker->collision;

        if (seeker->key == key)
            return seeker->value;
    }

    return -1;
}
