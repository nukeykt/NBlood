#include <stdint.h>
#include "opl3.h"

extern opl3_chip fm_chip;

#pragma pack(push,1)

typedef struct bank_tt
{
    uint8_t magicNumber[8];
    int16_t num_used_inst;
    int16_t num_stored_inst;
    uint32_t names_offset;
    uint32_t patch_offset;
} bank_t;

typedef struct bankname_tt
{
    uint8_t unused1;
    uint8_t unused2;
    uint8_t drum_note;
    char name[9];
} bankname_t;

typedef struct bankop_tt
{
    uint8_t ksl;
    uint8_t fmult;
    uint8_t feedback;
    uint8_t attack;
    uint8_t sustain;
    uint8_t eg;
    uint8_t decay;
    uint8_t release;
    uint8_t tl;
    uint8_t am;
    uint8_t vib;
    uint8_t ksr;
    uint8_t conn;
} bankop_t;

typedef struct bankdata_tt
{
    uint8_t unused1;
    uint8_t unused2;
    bankop_t mod;
    bankop_t car;
    uint8_t modWave;
    uint8_t carWave;
} bankdata_t;

#pragma pack(pop)

void MIDINoteOn(uint8_t note, uint8_t velocity, uint8_t channel);
void MIDINoteOff(int unused1, int unused2, uint8_t *msg);
void MIDISetPatch(int unused1, int unused2, uint8_t *msg);
void MIDICC(int unused1, int unused2, uint8_t *msg);
void MIDIBend(int unused1, int unused2, uint8_t *msg);
void MIDICCPan(uint32_t channel, uint8_t pan);
void MIDICCVolume(uint32_t channel, uint8_t volume);

int FMOPL3Reset(void);
int FMOPLReset(int);
int FMMute(void);
int FMOPL3Disable(void);

void WRITEPORT_6_28(uint32_t port, uint32_t reg, uint32_t data);
void WRITEPORT_10_19(uint32_t port, uint32_t reg, uint32_t data);

void FMConvertBank(bank_t *bank);

uint8_t FMGetVoice(uint8_t channel, uint8_t note);

uint8_t FMDisableVoice(uint8_t voice);

uint8_t FMSetupPatch(bankdata_t *patch, uint8_t voice);

uint32_t FMCalcBend(uint32_t bend, uint32_t note, uint32_t voice);
uint8_t FMEnableVoice(uint8_t voice, uint32_t freq);
uint8_t FMEnableVoiceBend(uint8_t voice, uint32_t freq);

uint8_t FMVoiceNote[9];
uint32_t FMVoiceChannel[9];
uint32_t FMVoiceVelocity[16] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127
};

uint32_t MIDIBendValue[16] =
{
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};
uint32_t MIDIDoBend[16];
uint32_t MIDIBendRange[16] =
{
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

uint32_t MIDIPatch[16];

uint32_t MIDIVolume[16] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127
};

uint32_t MIDIPan[16] =
{
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

uint8_t dword_12DC4[16];
uint8_t MIDIDoPan[16];
uint8_t MIDISustainedNotes[16];
uint8_t MIDISustain[16];
uint8_t MIDISustainList[16][32][3];

int fmInit;

int bankNum;

bank_t *melodic;
bankname_t *melodicNames;
bankdata_t *melodicPatches;
int melodicCount;
int melodicLoad;

bank_t *drum;
bankname_t *drumNames;
bankdata_t *drumPatches;
int drumLoad;

uint8_t FMCells[9][2] =
{
    {0, 3},
    {1, 4},
    {2, 5},
    {8, 11},
    {9, 12},
    {10, 13},
    {16, 19},
    {17, 20},
    {18, 21}
};

uint8_t FMTVDepth;
uint8_t OPL3Mode;

uint8_t F;

uint8_t FMHasPatch[11];


uint8_t FMVoiceKSL[32];

uint8_t FMSusRel[22]; // Orig had 16, a bug

uint8_t FMFreq[9];
uint8_t FMBlock[9];

uint8_t voltable[64] =
{
    0x3F, 0x3A, 0x35, 0x30, 0x2C, 0x29, 0x25, 0x24,
    0x23, 0x22, 0x21, 0x20, 0x1F, 0x1E, 0x1D, 0x1C,
    0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14,
    0x13, 0x12, 0x11, 0x10, 0x0F, 0x0E, 0x0E, 0x0D,
    0x0D, 0x0C, 0x0C, 0x0B, 0x0B, 0x0A, 0x0A, 0x09,
    0x09, 0x08, 0x08, 0x07, 0x07, 0x06, 0x06, 0x06,
    0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x03,
    0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x00,
};

uint32_t freqtable[] =
{
    0x0157, 0x016B, 0x0181, 0x0198, 0x01B0, 0x01CA, 0x01E5, 0x0202, 0x0220, 0x0241, 0x0263, 0x0287,
    0x0557, 0x056B, 0x0581, 0x0598, 0x05B0, 0x05CA, 0x05E5, 0x0602, 0x0620, 0x0641, 0x0663, 0x0687,
    0x0957, 0x096B, 0x0981, 0x0998, 0x09B0, 0x09CA, 0x09E5, 0x0A02, 0x0A20, 0x0A41, 0x0A63, 0x0A87,
    0x0D57, 0x0D6B, 0x0D81, 0x0D98, 0x0DB0, 0x0DCA, 0x0DE5, 0x0E02, 0x0E20, 0x0E41, 0x0E63, 0x0E87,
    0x1157, 0x116B, 0x1181, 0x1198, 0x11B0, 0x11CA, 0x11E5, 0x1202, 0x1220, 0x1241, 0x1263, 0x1287,
    0x1557, 0x156B, 0x1581, 0x1598, 0x15B0, 0x15CA, 0x15E5, 0x1602, 0x1620, 0x1641, 0x1663, 0x1687,
    0x1957, 0x196B, 0x1981, 0x1998, 0x19B0, 0x19CA, 0x19E5, 0x1A02, 0x1A20, 0x1A41, 0x1A63, 0x1A87,
    0x1D57, 0x1D6B, 0x1D81, 0x1D98, 0x1DB0, 0x1DCA, 0x1DE5, 0x1E02, 0x1E20, 0x1E41, 0x1E63, 0x1E87,
    0x1EAE, 0x1EB7, 0x1F02, 0x1F30, 0x1F60, 0x1F94, 0x1FCA
};

uint32_t bendtable[] =
{
    0x144, 0x132, 0x121, 0x110, 0x101, 0xf8, 0xe5, 0xd8, 0xcc, 0xc1, 0xb6, 0xac
}; // FIXME: more than 12?

void FMSendData(uint8_t *data)
{
    static uint8_t MIDIOffMsg[3];
    static uint8_t MIDIPatchMsg[2];
    static uint8_t MIDICCMsg[3];
    static uint8_t MIDIBendMsg[2];
    switch(data[0] & 0xf0)
    {
    case 0x90:
        if(data[2])
            MIDINoteOn(data[1], data[2], data[0] & 0x0f);
        else
        {
            MIDIOffMsg[2] = data[0] & 0x0f;
            MIDIOffMsg[0] = data[1];
            MIDIOffMsg[1] = data[2];
            MIDINoteOff(0, 0, MIDIOffMsg);
        }
        break;
    case 0x80:
        MIDIOffMsg[2] = data[0] & 0x0f;
        MIDIOffMsg[0] = data[1];
        MIDIOffMsg[1] = data[2];
        MIDINoteOff(0, 0, MIDIOffMsg);
        break;
    case 0xc0:
        MIDIPatchMsg[0] = data[1];
        MIDIPatchMsg[1] = data[0] & 0x0f;
        MIDISetPatch(0, 0, MIDIPatchMsg);
        break;
    case 0xb0:
        MIDICCMsg[0] = data[0] & 0x0f;
        MIDICCMsg[1] = data[1];
        MIDICCMsg[2] = data[2];
        MIDICC(0, 0, MIDICCMsg);
        break;
    case 0xe0:
        MIDIBendMsg[0] = data[0] & 0x0f;
        MIDIBendMsg[1] = data[2];
        MIDIBend(0, 0, MIDIBendMsg);
        break;
    }
}

int FMInit(void)
{
    FMOPL3Reset();
    FMOPLReset(0);
    fmInit = 1;
    return 0;
}

void FMUnInit(void)
{
    FMOPL3Reset();
    FMMute();
    FMOPL3Disable();
    fmInit = 0;
}

void FMReset(void)
{
    uint32_t i;
    FMMute();
    for(i = 0; i < 9; i++)
    {
        FMVoiceNote[i] = 0;
        FMVoiceChannel[i] = 0;
        FMVoiceVelocity[i] = 127;
    }
    for(i = 0; i < 16; i++)
    {
        MIDIBendValue[i] = 64;
        MIDIPatch[i] = 0;
        MIDIDoBend[i] = 0;
        MIDIVolume[i] = 127;
        MIDIPan[i] = 64;
        dword_12DC4[i] = 0;
        MIDIDoPan[i] = 0;
        MIDISustainedNotes[i] = 0;
    }
}

int FMSetBank(void *_bank)
{
    bank_t *bank = (bank_t*)_bank;
    static uint8_t MIDIPatchMsg[2];
    uint32_t i;
    if(bank->magicNumber[2] != 'H')
        FMConvertBank(bank);
    if(bankNum == 0)
    {
        bankNum = 1;
        melodic = bank;
        melodicCount = bank->num_used_inst;
        melodicNames = (bankname_t *)((uint8_t *)bank + bank->names_offset);
        melodicPatches = (bankdata_t *)((uint8_t *)bank + bank->patch_offset);
        melodicLoad = 1;
        for(i = 0; i < 9; i++)
        {
            MIDIPatchMsg[0] = 0;
            MIDIPatchMsg[1] = i;
            MIDISetPatch(i, 1000, MIDIPatchMsg);
        }
    }
    else
    {
        bankNum = 0;
        drum = bank;
        drumNames = (bankname_t *)((uint8_t *)bank + bank->names_offset);
        drumPatches = (bankdata_t *)((uint8_t *)bank + bank->patch_offset);
        drumLoad = 1;
    }
    return 0;
}

void FMConvertBank(bank_t *bank)
{
    bankdata_t *data;
    uint16_t i;
    uint32_t buf;
    bank->magicNumber[2] = 'H';
    data = (bankdata_t *)((uint8_t *)bank + bank->patch_offset);
    for(i = 0; i < bank->num_used_inst - 2; i++)
    {
        buf = (data->mod.am << 7) | (data->mod.vib << 6) | (data->mod.eg << 5) | (data->mod.ksr << 4) | data->mod.fmult;
        data->mod.am = buf;
        buf = (data->mod.ksl << 6) | data->mod.tl;
        data->mod.ksl = buf;
        buf = (data->mod.attack << 4) | data->mod.decay;
        data->mod.attack = buf;
        buf = (data->mod.sustain << 4) | data->mod.release;
        data->mod.sustain = buf;
        buf = (data->mod.feedback << 1) | data->mod.conn;
        data->mod.conn = buf;
        buf = (data->car.am << 7) | (data->car.vib << 6) | (data->car.eg << 5) | (data->car.ksr << 4) | data->car.fmult;
        data->car.am = buf;
        buf = (data->car.ksl << 6) | data->car.tl;
        data->car.ksl = buf;
        buf = (data->car.attack << 4) | data->car.decay;
        data->car.attack = buf;
        buf = (data->car.sustain << 4) | data->car.release;
        data->car.sustain = buf;
        data++;
    }
}

int dword_12C34 = 1;

void MIDINoteOn(uint8_t note, uint8_t velocity, uint8_t channel)
{
    uint32_t i, v10;
    uint8_t midichannel, fmvoice, vc;
    if(channel >= 16)
        return;

    if(channel != 9)
    {
        midichannel = channel;
        fmvoice = FMGetVoice(channel, note);
        FMDisableVoice(fmvoice);
        FMVoiceChannel[fmvoice] = channel;
        channel = fmvoice;
        for(i = 0; i < 5; i++)
        {
            vc = FMSusRel[FMCells[fmvoice][1]] | 0x0f;
            WRITEPORT_6_28(0, 0x80 + FMCells[fmvoice][1], vc);
            WRITEPORT_10_19(2, 0x80 + FMCells[fmvoice][1], vc);
            vc = FMSusRel[FMCells[fmvoice][0]] | 0x0f;
            WRITEPORT_6_28(0, 0x80 + FMCells[fmvoice][0], vc);
            WRITEPORT_10_19(2, 0x80 + FMCells[fmvoice][0], vc);
        }
        FMSetupPatch(melodicPatches + MIDIPatch[midichannel], channel);
        FMVoiceVelocity[channel] = velocity;
        velocity = (((MIDIVolume[midichannel] * 128) / 127) * FMVoiceVelocity[channel]) >> 7;
        if(melodicPatches[MIDIPatch[midichannel]].mod.conn & 1)
        {
            fmvoice = FMVoiceKSL[FMCells[channel][0]] & 63;
            v10 = voltable[velocity >> 1];
            v10 = 64 - v10;
            v10 <<= 1;
            v10 *= 64 - fmvoice;
            v10 = 8192 - v10;
            v10 >>= 7;
            fmvoice = (FMVoiceKSL[FMCells[channel][0]] & 0xc0) | v10;
            WRITEPORT_10_19(2, 0x40 + FMCells[channel][0], fmvoice);
            WRITEPORT_6_28(0, 0x40 + FMCells[channel][0], fmvoice);
        }
        fmvoice = FMVoiceKSL[FMCells[channel][1]] & 63;
        v10 = voltable[velocity >> 1];
        v10 = 64 - v10;
        v10 <<= 1;
        v10 *= 64 - fmvoice;
        v10 = 8192 - v10;
        v10 >>= 7;
        fmvoice = (FMVoiceKSL[FMCells[channel][1]] & 0xc0) | v10;
        WRITEPORT_10_19(2, 0x40 + FMCells[channel][1], fmvoice);
        WRITEPORT_6_28(0, 0x40 + FMCells[channel][1], fmvoice);
        FMVoiceNote[channel] = note;
        MIDICCPan(midichannel, MIDIPan[midichannel]);
        FMEnableVoice(channel, freqtable[note - 12]);

        if(dword_12C34 && MIDIDoBend[midichannel])
            FMEnableVoiceBend(channel, FMCalcBend(MIDIBendValue[midichannel], note, channel));
    }
    else
    {
        midichannel = channel;
        fmvoice = FMGetVoice(channel, note);
        FMDisableVoice(fmvoice);
        FMVoiceChannel[fmvoice] = channel;
        channel = fmvoice;
        for(i = 0; i < 5; i++)
        {
            vc = FMSusRel[FMCells[fmvoice][1]] | 0x0f;
            WRITEPORT_6_28(0, 0x80 + FMCells[fmvoice][1], vc);
            WRITEPORT_10_19(2, 0x80 + FMCells[fmvoice][1], vc);
            vc = FMSusRel[FMCells[fmvoice][0]] | 0x0f;
            WRITEPORT_6_28(0, 0x80 + FMCells[fmvoice][0], vc);
            WRITEPORT_10_19(2, 0x80 + FMCells[fmvoice][0], vc);
        }
        FMSetupPatch(drumPatches + note, channel);
        FMVoiceVelocity[channel] = velocity;
        velocity = (((MIDIVolume[midichannel] * 128) / 127) * FMVoiceVelocity[channel]) >> 7;
        if(drumPatches[note].mod.conn & 1)
        {
            fmvoice = FMVoiceKSL[FMCells[channel][0]] & 63;
            v10 = voltable[velocity >> 1];
            v10 = 64 - v10;
            v10 <<= 1;
            v10 *= 64 - fmvoice;
            v10 = 8192 - v10;
            v10 >>= 7;
            fmvoice = (FMVoiceKSL[FMCells[channel][0]] & 0xc0) | v10;
            WRITEPORT_10_19(2, 0x40 + FMCells[channel][0], fmvoice);
            WRITEPORT_6_28(0, 0x40 + FMCells[channel][0], fmvoice);
        }
        fmvoice = FMVoiceKSL[FMCells[channel][1]] & 63;
        v10 = voltable[velocity >> 1];
        v10 = 64 - v10;
        v10 <<= 1;
        v10 *= 64 - fmvoice;
        v10 = 8192 - v10;
        v10 >>= 7;
        fmvoice = (FMVoiceKSL[FMCells[channel][1]] & 0xc0) | v10;
        WRITEPORT_10_19(2, 0x40 + FMCells[channel][1], fmvoice);
        WRITEPORT_6_28(0, 0x40 + FMCells[channel][1], fmvoice);
        FMVoiceNote[channel] = note;
        MIDICCPan(midichannel, MIDIPan[midichannel]);
        FMEnableVoice(channel, freqtable[drumNames[note].drum_note - 12]);
    }
}

void MIDINoteOff(int unused1, int unused2, uint8_t *msg)
{
    uint32_t i;
    (void)unused1;
    (void)unused2;

    if(msg[2] >= 16)
        return;
    if(msg[2] < 16 || msg[2] == 9)
    {
        if(MIDISustain[msg[2]])
        {
            if(MIDISustainedNotes[msg[2]] < 32)
            {
                MIDISustainList[msg[2]][MIDISustainedNotes[msg[2]]][0] = msg[0];
                MIDISustainList[msg[2]][MIDISustainedNotes[msg[2]]][1] = msg[1];
                MIDISustainList[msg[2]][MIDISustainedNotes[msg[2]]][2] = msg[2];
                MIDISustainedNotes[msg[2]]++;
                return;
            }
        }
        for(i = 0; i < 9; i++)
        {
            if(FMVoiceNote[i] == msg[0] && msg[2] == FMVoiceChannel[i])
            {
                FMDisableVoice(i);
                FMVoiceNote[i] = 0;
            }
        }
    }
}

void MIDICCAllNotesOff(uint32_t channel)
{
    uint32_t i;
    if(channel < 16 || (channel == 9 && drumLoad))
    {
        for(i = 0; i < 9; i++)
        {
            if(FMVoiceChannel[i] == channel)
            {
                FMDisableVoice(i);
                FMVoiceNote[i] = 0;
            }
        }
    }
}

void MIDICCAllControllersOff(uint32_t channel)
{
    if(channel < 16 || (channel == 9 && drumLoad))
    {
        MIDIVolume[channel] = 127;
        FMVoiceVelocity[channel] = 127; // FIXME: overflow
        dword_12DC4[channel] = 0;
        MIDISustain[channel] = 0;
        MIDIBendValue[channel] = 64;
        MIDIBendRange[channel] = 2;
    }
}

void MIDISetPatch(int unused1, int unused2, uint8_t *msg)
{
    MIDIPatch[msg[1]] = msg[0];
}

void MIDICC(int unused1, int unused2, uint8_t *msg)
{
    switch(msg[1])
    {
    case 7:
        MIDICCVolume(msg[0], msg[2]);
        break;
    case 10:
        MIDICCPan(msg[0], msg[2]);
        break;
    case 123:
        MIDICCAllNotesOff(msg[0]);
        break;
    case 121:
        MIDICCAllControllersOff(msg[0]);
        break;
    case 64:
        MIDISustain[msg[0]] = msg[2];
        if(!msg[2])
        {
            while(MIDISustainedNotes[msg[0]])
            {
                MIDISustainedNotes[msg[0]]--;
                MIDINoteOff(0, 0, MIDISustainList[msg[0]][MIDISustainedNotes[msg[0]]]);
            }
        }
        break;
    case 102:
        MIDIBendRange[msg[0]] = msg[2];
        break;
    }
}

void MIDIBend(int unused1, int unused2, uint8_t *msg)
{
    uint32_t i;
    if(msg[0] >= 16)
        return;
    if(msg[0] != 9)
    {
        MIDIBendValue[msg[0]] = msg[1];
        MIDIDoBend[msg[0]] = 1;
        for(i = 0; i < 9; i++)
        {
            if(FMVoiceNote[i])
            {
                if(msg[0] == FMVoiceChannel[i])
                    FMEnableVoiceBend(i, FMCalcBend(msg[1], FMVoiceNote[i], i));
            }
        }
    }
}

uint32_t FMCalcBend(uint32_t bend, uint32_t note, uint32_t voice)
{
    uint32_t noteMod12, bendFactor, outFreq, fmOctave, fmFreq, newFreq;
    note -= 12;

    noteMod12 = note;
    while(noteMod12 >= 12)
        noteMod12 -= 12;

    outFreq = freqtable[note];
    fmOctave = outFreq & 0x1c00;
    fmFreq = outFreq & 0x3ff;

    if(bend < 64)
    {
        bendFactor = ((63 - bend) * 1000) >> 6;
        newFreq = outFreq - freqtable[note - MIDIBendRange[voice]];
        if(newFreq > 719)
        {
            newFreq = fmFreq - bendtable[MIDIBendRange[FMVoiceChannel[voice]] - 1];
            newFreq &= 0x3ff;
        }
        newFreq = (newFreq * bendFactor) / 1000;
        outFreq -= newFreq;
    }
    else
    {
        bendFactor = ((bend - 64) * 1000) >> 6;
        newFreq = freqtable[note + MIDIBendRange[FMVoiceChannel[voice]]] - outFreq;
        if(newFreq > 719)
        {
            fmFreq = bendtable[11 - noteMod12];
            outFreq = (fmOctave + 1024) | fmFreq;
            newFreq = freqtable[note + MIDIBendRange[FMVoiceChannel[voice]]] - outFreq;
        }
        newFreq = (newFreq * bendFactor) / 1000;
        outFreq += newFreq;
    }

    return outFreq;
}

int FMOPL3Reset(void)
{
    WRITEPORT_10_19(2, 5, 1);
    WRITEPORT_10_19(2, 4, 0);
    FMTVDepth = 0;
    FMOPLReset(0);
    OPL3Mode = 1;
    return 0;
}

int FMOPLReset(int a1)
{
    uint32_t i;
    for(i = 0; i < 9; i++)
    {
        FMBlock[i] = 0;
        WRITEPORT_6_28(0, 0xb0 + i, FMBlock[i]);
        WRITEPORT_10_19(2, 0xb0 + i, FMBlock[i]);
    }
    for(i = 0; i < 11; i++)
        FMHasPatch[i] = 0;
    FMTVDepth &= 0xc0;
    FMTVDepth |= 0xc0;
    WRITEPORT_6_28(0, 0xbd, FMTVDepth);
    return 0;
}

void WRITEPORT_6_28(uint32_t port, uint32_t reg, uint32_t data)
{
    OPL3_WriteRegBuffered(&fm_chip, reg + (port << 7), data);
    // Write port+0, reg
    // Read port+1, x6
    // Write port+1, data
    // Read port+0, x28
}

void WRITEPORT_10_19(uint32_t port, uint32_t reg, uint32_t data)
{
    OPL3_WriteRegBuffered(&fm_chip, reg + (port << 7), data);
    // Write port+0, reg
    // Read port+1, x10
    // Write port+1, data
    // Read port+0, x19
}

int FMMute(void)
{
    uint32_t i;
    if(!OPL3Mode)
        return 2;
    FMTVDepth = 0;
    WRITEPORT_6_28(0, 0xbd, FMTVDepth);
    for(i = 0; i < 9; i++)
    {
        WRITEPORT_6_28(0, 0xb0 + i, FMBlock[i] & ~0x20);
        WRITEPORT_10_19(2, 0xb0 + i, FMBlock[i] & ~0x20);
    }
    for(i = 0; i < 9; i++)
    {
        WRITEPORT_6_28(0, 0x40 + FMCells[i][1], 0xff);
        WRITEPORT_10_19(2, 0x40 + FMCells[i][1], 0xff);
    }
    for(i = 0; i < 11; i++)
        FMHasPatch[i] = 0;
    return 0;
}

int FMOPL3Disable(void)
{
    WRITEPORT_10_19(2, 5, 0);
    OPL3Mode = 0;
    return 0;
}

uint8_t FMGetVoice(uint8_t channel, uint8_t note)
{
    uint32_t i, j;
    for(i = 0; i < 9; i++)
    {
        if(!FMVoiceNote[i])
            return i;
    }
    for(i = 0; i < 16; i++)
    {
        if(!MIDIDoBend[i])
        {
            for(j = 0; j < 9; j++)
            {
                if(FMVoiceChannel[j] == i)
                    return j;
            }
        }
    }
    if(channel >= 9)
        channel -= 9;
    return channel;
}

uint8_t FMDisableVoice(uint8_t voice)
{
    if(!FMVoiceNote[voice])
        return 6;
    FMBlock[voice] &= ~0x20;
    WRITEPORT_6_28(0, 0xb0 + voice, FMBlock[voice]);
    WRITEPORT_10_19(2, 0xb0 + voice, FMBlock[voice]);
    FMVoiceNote[voice] = 0;
    return 0;
}

uint8_t FMSetupPatch(bankdata_t *patch, uint8_t voice)
{
    uint32_t v18, v10, vc, v34, v14, v38, v1c, v20, v24, v28, v2c, v30;

    v18 = FMCells[voice][0];
    v10 = patch->mod.am;
    vc = patch->mod.ksl;
    v34 = patch->mod.attack;
    v14 = patch->mod.sustain;
    v38 = patch->mod.conn;
    v1c = patch->modWave;
    v20 = patch->car.am;
    v24 = patch->car.ksl;
    v28 = patch->car.attack;
    v2c = patch->car.sustain;
    v30 = patch->carWave;
    FMSusRel[FMCells[voice][1]] = v2c;
    FMSusRel[FMCells[voice][0]] = v14;
    WRITEPORT_6_28(0, 0x20 + v18, v10);
    WRITEPORT_10_19(2, 0x20 + v18, v10);
    WRITEPORT_6_28(0, 0x40 + v18, vc);
    WRITEPORT_10_19(2, 0x40 + v18, vc);
    FMVoiceKSL[v18] = vc;
    WRITEPORT_6_28(0, 0x60 + v18, v34);
    WRITEPORT_10_19(2, 0x60 + v18, v34);
    WRITEPORT_6_28(0, 0x80 + v18, v14);
    WRITEPORT_10_19(2, 0x80 + v18, v14);
    v38 &= 0x1f;
    v38 |= 0x20;
    WRITEPORT_6_28(0, 0xc0 + voice, v38);
    v38 &= ~0x20;
    v38 |= 0x10;
    WRITEPORT_10_19(2, 0xc0 + voice, v38);
    WRITEPORT_6_28(0, 0xe0 + v18, v1c);
    WRITEPORT_10_19(2, 0xe0 + v18, v1c);
    v18 = FMCells[voice][1];
    WRITEPORT_6_28(0, 0x20 + v18, v20);
    WRITEPORT_10_19(2, 0x20 + v18, v20);
    WRITEPORT_6_28(0, 0x40 + v18, v24);
    WRITEPORT_10_19(2, 0x40 + v18, v24);
    FMVoiceKSL[v18] = v24;
    WRITEPORT_6_28(0, 0x60 + v18, v28);
    WRITEPORT_10_19(2, 0x60 + v18, v28);
    WRITEPORT_6_28(0, 0x80 + v18, v2c);
    WRITEPORT_10_19(2, 0x80 + v18, v2c);
    WRITEPORT_6_28(0, 0xe0 + v18, v30);
    WRITEPORT_10_19(2, 0xe0 + v18, v30);
    FMHasPatch[voice] = 1;
    return 0;
}

void MIDICCVolume(uint32_t channel, uint8_t volume)
{
    uint32_t i, v10, vc;
    uint8_t v4;
    if(channel >= 16)
        return;
    MIDIVolume[channel] = volume;
    dword_12DC4[channel] = 1;
    for(i = 0; i < 9; i++)
    {
        if(FMVoiceNote[i])
        {
            if(FMVoiceChannel[i] == channel)
            {
                v4 = (((volume * 128) / 127) * FMVoiceVelocity[i]) >> 7;
                if(melodicPatches[MIDIPatch[FMVoiceChannel[i]]].mod.conn & 1)
                {
                    v10 = FMVoiceKSL[FMCells[i][0]] & 63;
                    vc = voltable[v4 >> 1];
                    vc = 64 - vc;
                    vc = (vc << 7) >> 6;
                    vc *= 64 - v10;
                    vc = 8192 - vc;
                    vc >>= 7;
                    v10 = (FMVoiceKSL[FMCells[i][0]] & 0xc0) | vc;
                    if(MIDIPan[channel] < 64)
                        WRITEPORT_6_28(0, 0x40 + FMCells[i][0], v10);
                    else
                        WRITEPORT_10_19(2, 0x40 + FMCells[i][0], v10);
                }
                v10 = FMVoiceKSL[FMCells[i][1]] & 63;
                vc = voltable[v4 >> 1];
                vc = 64 - vc;
                vc = (vc << 7) >> 6;
                vc *= 64 - v10;
                vc = 8192 - vc;
                vc >>= 7;
                v10 = (FMVoiceKSL[FMCells[i][1]] & 0xc0) | vc;
                if(MIDIPan[channel] < 64)
                    WRITEPORT_6_28(0, 0x40 + FMCells[i][1], v10);
                else
                    WRITEPORT_10_19(2, 0x40 + FMCells[i][1], v10);
            }
        }
    }
    MIDICCPan(channel, MIDIPan[channel]);
}

void MIDICCPan(uint32_t channel, uint8_t pan)
{
    uint32_t i, v10, vc;
    uint8_t v4;
    MIDIPan[channel] = pan;
    MIDIDoPan[channel] = 1;
    if(pan >= 64)
        pan = 127 - pan;
    pan <<= 1;
    for(i = 0; i < 9; i++)
    {
        if(FMVoiceNote[i])
        {
            if(FMVoiceChannel[i] == channel)
            {
                v4 = (MIDIVolume[channel] * FMVoiceVelocity[i]) >> 7;
                v4 = (v4 * pan) >> 7;
                v10 = FMVoiceKSL[FMCells[i][1]] & 63;
                vc = voltable[v4 >> 1];
                vc = 64 - vc;
                vc = (vc << 7) >> 6;
                vc *= 64 - v10;
                vc = 8192 - vc;
                vc >>= 7;
                v10 = (FMVoiceKSL[FMCells[i][1]] & 0xc0) | vc;
                if(MIDIPan[channel] < 64)
                    WRITEPORT_10_19(2, 0x40 + FMCells[i][1], v10);
                else
                    WRITEPORT_6_28(0, 0x40 + FMCells[i][1], v10);
                if(melodicPatches[MIDIPatch[FMVoiceChannel[i]]].mod.conn & 1)
                {
                    v10 = FMVoiceKSL[FMCells[i][0]] & 63;
                    vc = voltable[v4 >> 1];
                    vc = 64 - vc;
                    vc = (vc << 7) >> 6;
                    vc *= 64 - v10;
                    vc = 8192 - vc;
                    vc >>= 7;
                    v10 = (FMVoiceKSL[FMCells[i][0]] & 0xc0) | vc;
                    if(MIDIPan[channel] < 64)
                        WRITEPORT_10_19(2, 0x40 + FMCells[i][0], v10);
                    else
                        WRITEPORT_6_28(0, 0x40 + FMCells[i][0], v10);
                }
            }
        }
    }
}

uint8_t FMEnableVoice(uint8_t voice, uint32_t freq)
{
    FMFreq[voice] = freq & 0xff;
    FMBlock[voice] = (freq >> 8) | 0x20;
    WRITEPORT_6_28(0, 0xa0 + voice, FMFreq[voice]);
    WRITEPORT_10_19(2, 0xa0 + voice, FMFreq[voice]);
    WRITEPORT_6_28(0, 0xb0 + voice, FMBlock[voice] & ~0x20);
    WRITEPORT_10_19(2, 0xb0 + voice, FMBlock[voice] & ~0x20);
    WRITEPORT_6_28(0, 0xb0 + voice, FMBlock[voice]);
    WRITEPORT_10_19(2, 0xb0 + voice, FMBlock[voice]);
    return 0;
}

uint8_t FMEnableVoiceBend(uint8_t voice, uint32_t freq)
{
    FMFreq[voice] = freq & 0xff;
    FMBlock[voice] = (freq >> 8) | 0x20;
    WRITEPORT_6_28(0, 0xa0 + voice, FMFreq[voice]);
    WRITEPORT_10_19(2, 0xa0 + voice, FMFreq[voice]);
    WRITEPORT_6_28(0, 0xb0 + voice, FMBlock[voice]);
    WRITEPORT_10_19(2, 0xb0 + voice, FMBlock[voice]);
    return 0;
}
