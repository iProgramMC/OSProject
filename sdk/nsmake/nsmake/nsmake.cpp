#include "main.h"

float GetVersion()
{
    return /*V*/0.10f;
}

int main(int argc, char** argv)
{
    LogMsg("NanoShell Make V%.2f", GetVersion());
    for (int i = 0; i < argc; i++)
    {
        LogMsg("Got argument: %s", argv[i]);
    }
}