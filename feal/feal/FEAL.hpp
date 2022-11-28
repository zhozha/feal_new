#include "FEAL.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#define ROUND 8

uint8_t S1(uint8_t a, uint8_t b) //+
{
    return (((a + b + 1) % 256) << 2) | (((a + b + 1) % 256) >> 6);
}

uint8_t S0(uint8_t a, uint8_t b) //+
{
    return (((a + b) % 256) << 2) | (((a + b) % 256) >> 6);
}

FEAL::Key catKey(uint8_t SubKeys[8]) 
{
    FEAL::Key Temp;
    Temp = 0x00;

    for (int i = 0; i < 3; i++)
    {
        Temp = Temp | SubKeys[i];
        Temp = Temp << 8;
    }
    Temp = Temp | SubKeys[3];
    return Temp;
}

void setSubKeys(FEAL::Key key1, FEAL::Key key2, uint8_t SubKeys[8]) //+
{
    for (int i = 3; i >= 0; i--)
    {
        SubKeys[i] = (uint8_t)(key1 & 0xFF);
        key1 = key1 >> 8;
    }

    for (int i = 7; i >= 4; i--)
    {
        SubKeys[i] = (uint8_t)(key2 & 0xFF);
        key2 = key2 >> 8;
    };
}

FEAL::Key Fkey(FEAL::Key key1, FEAL::Key key2) //+
{
    uint8_t SubKeys[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    setSubKeys(key1, key2, SubKeys);
    SubKeys[1] = SubKeys[1] ^ SubKeys[0];
    SubKeys[2] = SubKeys[2] ^ SubKeys[3];
    SubKeys[4] = SubKeys[4] ^ SubKeys[2];
    SubKeys[1] = S1(SubKeys[1], SubKeys[4]);
    SubKeys[5] = SubKeys[5] ^ SubKeys[1];
    SubKeys[2] = S0(SubKeys[2], SubKeys[5]);
    SubKeys[6] = SubKeys[6] ^ SubKeys[1];
    SubKeys[0] = S0(SubKeys[0], SubKeys[6]);
    SubKeys[7] = SubKeys[7] ^ SubKeys[2];
    SubKeys[3] = S1(SubKeys[3], SubKeys[7]);
    return catKey(SubKeys);
}

void generateKeys(FEAL::Key key[2], FEAL::Keys keys[ 16 ]) //+
{
    FEAL::Key Left = 0x00, Right = 0x00, TempKey = 0x00, TempKey2 = 0x00, Temp = 0x00;
    Left = key[0]; Right = key[1];
    for (int i = 0; i < ROUND; i++)
    {
        TempKey = Left;
        if (i == 0) Left = Fkey(Left, Right);
            else Left = Fkey(Left, Right ^ TempKey2);
        keys[i * 2] = Left >> 16;
        keys[i * 2 + 1] = (FEAL::Keys) Left & 0xFFFF;
        Temp = Left;
        Left = Right;
        Right = Temp;
        TempKey2 = TempKey;
    }
}

void getInvKeys(FEAL::Keys keys[ 16 ])
{
    FEAL::Keys temp = 0x0;

    for (int i = 0; i < 4; i++)
    {
        temp = keys[i];
        keys[i] = keys[7 - i];
        keys[7 - i] = temp;
    }

    for (int i = 8; i < 12; i++)
    {
        temp = keys[i];
        keys[i] = keys[4 + i];
        keys[4 + i] = temp;
    }
}

FEAL::Message Fproc(FEAL::Message msg, FEAL::Keys key) //+
{
    uint8_t SubMsg[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, SubKeys[2] = {0x0, 0x0};

    for (int i = 3; i >= 0; i--)
    {
        SubMsg[i] = (uint8_t)msg & 0xFF;
        msg = msg >> 8;
    }
    for (int i = 1; i >= 0; i--)
    {
        SubKeys[i] = (uint8_t)key & 0xFF;
        key = key >> 8;
    }

    SubMsg[1] = SubMsg[1] ^ SubKeys[0];
    SubMsg[2] = SubMsg[2] ^ SubKeys[1];
    SubMsg[1] = SubMsg[1] ^ SubMsg[0];
    SubMsg[2] = SubMsg[2] ^ SubMsg[3];
    SubMsg[1] = S1(SubMsg[1], SubMsg[2]);
    SubMsg[2] = S0(SubMsg[2], SubMsg[1]);
    SubMsg[0] = S0(SubMsg[0], SubMsg[1]);
    SubMsg[3] = S1(SubMsg[3], SubMsg[2]); 

    return catKey(SubMsg);
}

void XORMSG(FEAL::Message msg[2], FEAL::Keys K0, FEAL::Keys K1, FEAL::Keys K2, FEAL::Keys K3) //+
{
    msg[0] = msg[0] ^ (((FEAL::Message) K0 & 0xFFFF) << 16);
    msg[0] = msg[0] ^ (((FEAL::Message) K1 & 0xFFFF));
    msg[1] = msg[1] ^ (((FEAL::Message) K2 & 0xFFFF) << 16);
    msg[1] = msg[1] ^ (((FEAL::Message) K3 & 0xFFFF));
}

void FEAL::encrypt( FEAL::Key key[2], FEAL::Message msg[2])
{
    run(key, msg, false);
}

void FEAL::decrypt( FEAL::Key key[2], FEAL::Message msg[2])
{
    run(key, msg, true);
}

void FEAL::run(FEAL::Key key[2], FEAL::Message msg[2], bool decryption)
{
    static FEAL::Keys keys[ FEAL::KEY_LEN ] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    static FEAL::Message Left = 0x0, Right = 0x0, Temp = 0x0;

    generateKeys(key, keys); 

    if (decryption)
        getInvKeys(keys);
    XORMSG(msg, keys[8], keys[9], keys[10], keys[11]); 

    Left = msg[0]; Right = msg[1]; 
    Right = Right ^ Left; 

    for (int i = 0; i < ROUND; i++) 
    {
        Left = Left ^ Fproc(Right, keys[i]);
        if (i == ROUND - 1) continue;
        Temp = Left; 
        Left = Right; 
        Right = Temp; 
    }

    Right = Right ^ Left;
    msg[0] = Left; msg[1] = Right;
    XORMSG(msg, keys[12], keys[13], keys[14], keys[15]);
}
