#include "compat.h"
#include "reality.h"
#include "reality_sound.h"

rt_CTL_t *soundCtl, *musicCtl;

static rt_instrument_t *RT_LoadInstrument(uint32_t ctlOffset, uint32_t instOffset, uint32_t tblOffset)
{
    return nullptr;
}

static rt_bank_t *RT_LoadBank(uint32_t ctlOffset, uint32_t bankOffset, uint32_t tblOffset)
{
    rt_bank_t *bank = (rt_bank_t*)Xcalloc(1, sizeof(rt_bank_t));
    lseek(rt_group, bankOffset, SEEK_SET);
    read(rt_group, &bank->inst_count, sizeof(bank->inst_count));
    bank->inst_count = B_BIG16(bank->inst_count);
    read(rt_group, &bank->flags, sizeof(bank->flags));
    bank->flags = B_BIG16(bank->flags);
    read(rt_group, &bank->unused, sizeof(bank->unused));
    bank->unused = B_BIG16(bank->unused);
    read(rt_group, &bank->rate, sizeof(bank->rate));
    bank->rate = B_BIG16(bank->rate);
    uint32_t percOffset, *instOffset;
    read(rt_group, &percOffset, sizeof(percOffset));
    percOffset = B_BIG32(percOffset);
    instOffset = (uint32_t*)Xcalloc(bank->inst_count, sizeof(uint32_t));
    read(rt_group, instOffset, sizeof(uint32_t) * bank->inst_count);

    bank->inst = (rt_instrument_t**)Xcalloc(1, sizeof(rt_instrument_t*));

    bank->perc = RT_LoadInstrument(ctlOffset, percOffset, tblOffset);
    for (int i = 0; i < bank->inst_count; i++)
    {
        bank->inst[i] = RT_LoadInstrument(ctlOffset, B_BIG32(instOffset[i]), tblOffset);
    }

    Xfree(instOffset);

    return bank;
}

rt_CTL_t *RT_LoadCTL(uint32_t ctlOffset, uint32_t tblOffset)
{
    rt_CTL_t *ctl = (rt_CTL_t*)Xcalloc(1, sizeof(rt_CTL_t));
    lseek(rt_group, ctlOffset, SEEK_SET);
    read(rt_group, &ctl->signature, sizeof(ctl->signature));
    ctl->signature = B_BIG16(ctl->signature);
    read(rt_group, &ctl->bank_count, sizeof(ctl->bank_count));
    ctl->bank_count = B_BIG16(ctl->bank_count);
    if (ctl->signature != signature || ctl->bank_count <= 0)
    {
        initprintf("Error loading sound bank %x\n", ctlOffset);
        Xfree(ctl);
        return nullptr;
    }
    uint32_t *bank_offset = (uint32_t*)Xcalloc(ctl->bank_count, sizeof(uint32_t));
    read(rt_group, bank_offset, sizeof(uint32_t) * ctl->bank_count);
    // Load banks
    for (int i = 0; i < ctl->bank_count; i++)
    {
        ctl->bank[i] = RT_LoadBank(ctlOffset, B_BIG32(bank_offset[i]), tblOffset);
    }
    Xfree(bank_offset);
}

