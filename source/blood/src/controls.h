#pragma once

union BUTTONFLAGS
{
    int8_t byte;
    struct
    {
        unsigned int jump : 1;
        unsigned int crouch : 1;
        unsigned int shoot : 1;
        unsigned int shoot2 : 1;
        unsigned int lookUp : 1;
        unsigned int lookDown : 1;
    };
};

union KEYFLAGS
{
    int16_t word;
    struct
    {
        unsigned int action : 1;
        unsigned int jab : 1;
        unsigned int prevItem : 1;
        unsigned int nextItem : 1;
        unsigned int useItem : 1;
        unsigned int prevWeapon : 1;
        unsigned int nextWeapon : 1;
        unsigned int holsterWeapon : 1;
        unsigned int lookCenter : 1;
        unsigned int lookLeft : 1;
        unsigned int lookRight : 1;
        unsigned int spin180 : 1;
        unsigned int pause : 1;
        unsigned int quit : 1;
        unsigned int restart : 1;
    };
};

union USEFLAGS
{
    uint8_t byte;
    struct
    {
        unsigned int useBeastVision : 1;
        unsigned int useCrystalBall : 1;
        unsigned int useJumpBoots : 1;
        unsigned int useMedKit : 1;
    };
};

union SYNCFLAGS
{
    uint8_t byte;
    struct
    {
        unsigned int buttonChange : 1;
        unsigned int keyChange : 1;
        unsigned int useChange : 1;
        unsigned int weaponChange : 1;
        unsigned int mlookChange : 1;
        unsigned int run : 1;
    };
};
struct GINPUT
{
    SYNCFLAGS syncFlags;
    int8_t forward;
    int16_t turn;
    int8_t strafe;
    BUTTONFLAGS buttonFlags;
    KEYFLAGS keyFlags;
    USEFLAGS useFlags;
    uint8_t newWeapon;
    int8_t mlook;
};

extern GINPUT gInput;
extern bool bSilentAim;
extern bool gMouseAim;

void ctrlInit();
void ctrlGetInput();

