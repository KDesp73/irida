#include "uci.h"

int main(int argc, char** argv)
{
    InitZobrist();
    InitMasks();

    return UciMain();
}
