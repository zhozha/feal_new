#include <stdint.h>

class FEAL
{
    public:
        enum
        {
            KEY_LEN = 16,
            BLOCKS_LEN = 2
        };
        typedef uint32_t Key;
        typedef uint16_t Keys;
        typedef uint32_t Message;

        void run( Key key[2], Message msg[2], bool reverse = false);
        void encrypt( Key key[2], Message msg[2] );
        void decrypt( Key key[2], Message msg[2] );
};