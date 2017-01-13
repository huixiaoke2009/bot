
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "bot.h"

int main(int argc, char* argv[])
{
    CBot carrot;
    int Ret = carrot.Init();
    if(Ret != 0)
    {
        printf("Init failed, Ret=%d\n", Ret);
        return -1;
    }
    
    Ret = carrot.Run();
    if(Ret != 0)
    {
        printf("Run failed, Ret=%d\n", Ret);
        return -1;
    }

    carrot.Finish();
    
    return 0;
}