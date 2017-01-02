
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "carrot.h"

int main(int argc, char* argv[])
{    
    CCarrot carrot;
    int Ret = carrot.Init();
    if(Ret != 0)
    {
        printf("Init failed, Ret=%d\n", Ret);
        return -1;
    }
    
    carrot.Run();

    carrot.Finish();
    
    return 0;
}