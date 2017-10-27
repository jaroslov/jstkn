#include "jstkn.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct STATE
{
};

void* Begin(JSTKNSideEffect*, JSTKNTypes, const char* ptr)
{
    return (void*)ptr;
}

int End(JSTKNSideEffect*, void* beg, const char* end)
{
    //fprintf(stdout, "    %.*s\n", (int)(end - (const char*)beg), (const char*)beg);
    return 0;
}

int Fail(JSTKNSideEffect*, void* beg, const char* end)
{
    fprintf(stdout, "    FAILED! %.*s\n", (int)(end - (const char*)beg), (const char*)beg);
    return 0;
}

void* Malloc(JSTKNSideEffect* SE, uint64_t NumBytes)
{
    return malloc(NumBytes);
}

void* Realloc(JSTKNSideEffect* SE, void* Orig, uint64_t NumBytes)
{
    return realloc(Orig, NumBytes);
}

int Free(JSTKNSideEffect* SE, void* addr, uint64_t NumBytes)
{
    free(addr);
    return 1;
}

int Schema(JSTKNSideEffect*, JSTKNTypes, void*)
{
    return 1;
}

int main(int argc, char *argv[])
{
    STATE S             = { };
    JSTKNSideEffect SE       =
    {
        .Handle         = &S,
        .Begin          = &Begin,
        .End            = &End,
        .Fail           = &Fail,
        .Malloc         = &Malloc,
        .Realloc        = &Realloc,
        .Free           = &Free,
        .Schema         = &Schema,
    };

    for (int AA = 1; AA < argc; ++AA)
    {
        FILE* fp        = fopen(argv[AA], "r");
        if (!fp)
        {
            return 1;
        }

        fseek(fp, 0, SEEK_END);
        long length     = ftell(fp);
        rewind(fp);

        char* jsonstr   = (char*)calloc(1, length);
        if (!jsonstr)
        {
            return 1;
        }
        fread(jsonstr, 1, length, fp);
        fclose(fp);

        uint32_t* json  = (uint32_t*)calloc(sizeof(uint32_t), length * 6);
        if (!json)
        {
            return 1;
        }

        int J           = 1;
        for (int II = 0; II < 1; ++II)
        {
            J           += jstknParse(jsonstr, jsonstr + length, &SE);
        }
        fprintf(stdout, "Parsed? %s '%s'\n",
            (J)
                ? "yes"
                : "no ",
                argv[AA]);

        if (json)
        {
            free(json);
        }
        if (jsonstr)
        {
            free(jsonstr);
        }
    }

    return 0;
}

#define JSTKN_C_SOURCE
#include "jstkn.h"
#undef  JSTKN_C_SOURCE
