#include "FEAL.hpp"
#include <stdio.h>
#include <iostream>
#include <iomanip>

using namespace std;

void showComponent(uint32_t x)
{
    cout << hex << setw(8) << setfill('0') << uppercase << x << " ";
}

void showMessage(const FEAL::Message msg[2])
{
    for (int i = 0; i < FEAL::BLOCKS_LEN; ++i)
    {
        showComponent(msg[i]);
        cout << " ";
    }
    cout << endl;
}

int main()
{
    FEAL feal;
    static FEAL::Key KEY[2] = { 0x01234567, 0x89ABCDEF };
    FEAL::Message msg[2] =    { 0x0001023123123A, 0x0DFE32133EEFD };
    cout << "Key: ";
    showMessage(KEY);
    cout << "Msg before: ";
    showMessage(msg);
    feal.encrypt(KEY, msg);
    cout << "Msg after encryption: ";
    showMessage(msg);
    feal.decrypt(KEY, msg);
    cout << "Msg after decryption: ";
    showMessage(msg);
    cout << endl;
    return 0;
}