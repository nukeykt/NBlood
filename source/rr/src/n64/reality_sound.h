#pragma once

#include "compat.h"

#pragma pack(push, 1)

const uint16_t signature = 0x4231; // B1

struct rt_adpcm_book_t {
    uint32_t order;
    uint32_t npredictors;
    int16_t *predictors;
};

struct rt_adpcm_loop_t {
    uint32_t start;
    uint32_t end;
    uint32_t count;
    uint16_t state[16];
    uint32_t unknown1;
};

struct rt_raw_loop_t {
    uint32_t start;
    uint32_t end;
    uint32_t count;
};

struct rt_adpcm_wave_t {
    rt_adpcm_loop_t *loop;
    rt_adpcm_book_t *book;
};

struct rt_raw_wave_t {
    rt_raw_loop_t *loop;
};

struct rt_wave_t {
    uint32_t base;
    uint32_t len;
    uint8_t type;
    uint8_t flags;
    rt_adpcm_wave_t *adpcm;
    rt_raw_wave_t *raw;
};

struct rt_env_t {
    uint32_t attack_time;
    uint32_t decay_time;
    uint32_t release_time;
    uint8_t attack_volume;
    uint8_t decay_volume;
    uint16_t pad;
};

struct rt_key_t {
    uint8_t velocitymin;
    uint8_t velocitymax;
    uint8_t keymin;
    uint8_t keymax;
    uint8_t keybase;
    uint8_t detune;
};

struct rt_sound_t {
    rt_env_t *env;
    rt_key_t *key;
    rt_wave_t *wave;
    uint8_t sample_pan;
    uint8_t sample_volume;
    uint16_t flags;
};

struct rt_instrument_t {
    uint8_t volume;
    uint8_t pan;
    uint8_t priority;
    uint8_t flags;
    uint8_t trem_type;
    uint8_t trem_rate;
    uint8_t trem_depth;
    uint8_t trem_delay;
    uint8_t vib_type;
    uint8_t vib_rate;
    uint8_t vib_depth;
    uint8_t vib_delay;
    int16_t bend_range;
    int16_t sound_count;
    rt_sound_t **sounds;
};

struct rt_bank_t {
    int16_t inst_count;
    int16_t flags;
    int16_t unused;
    int16_t rate;
    rt_instrument_t *perc;
    rt_instrument_t **inst;
};

struct rt_CTL_t {
    uint16_t signature;
    uint16_t bank_count;
    rt_bank_t **bank;
};
#pragma pack(pop)

// extern rt_CTL_t *soundCtl, *musicCtl;
// 
// rt_CTL_t *RT_LoadCTL(uint32_t ctlOffset, uint32_t tblOffset);
void RT_InitSound(void);
int RT_LoadSound(int num);

