#ifndef JSTKN_H
#define JSTKN_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum JSTKNTypes
{
    JSTKN_Unknown,
    JSTKN_Any,
    JSTKN_Null,
    JSTKN_True,
    JSTKN_False,
    JSTKN_Number,
    JSTKN_String,
    JSTKN_Array,
    JSTKN_Object,
} JSTKNTypes;

typedef struct JSTKNSideEffect
{
    void*       Handle;
    void*     (*Begin)(JSTKNSideEffect*, JSTKNTypes, const char* ptr);
    int       (*End)(JSTKNSideEffect*, void*, const char* ptr);
    int       (*Fail)(JSTKNSideEffect*, void*, const char* ptr);

    void*     (*Malloc)(JSTKNSideEffect*, unsigned long long);
    int       (*Free)(JSTKNSideEffect*, void*, unsigned long long);
} JSTKNSideEffect;

int jstknParse(const char* fst, const char* lst, JSTKNSideEffect* SE);

#ifdef __cplusplus
}
#endif

#endif//JSTKN_H

#ifdef  JSTKN_C_SOURCE

typedef unsigned long long  uint64_j;
typedef signed long long    int64_j;

#define CHAR_BIT_J          (8)

static inline int jstknParseNull(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseTrue(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseFalse(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseNumber(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseString(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseArrayBegin(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseArrayEnd(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseObjectBegin(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseObjectEnd(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseColon(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);
static inline int jstknParseComma(void** Thing, const char **fst, const char *lst, long long* StkPos, unsigned long long* Stack, int* key, JSTKNSideEffect* SE);

static inline int jsspace(char c)
{
    return !!((1ULL << c) & ((1ULL << 0x09) | (1ULL << 0x0A) | (1ULL << 0x0B) | (1ULL << 0x0C) | (1ULL << 0x0D) | (1ULL << 0x20)));
}

int jstknParse(const char* fst, const char* lst, JSTKNSideEffect* SE)
{
    const char* cur     = fst;
    int result          = 0;
    uint64_j stklen     = (((lst - fst + (2 * CHAR_BIT_J - 1)) / (2 * CHAR_BIT_J)) + sizeof(uint64_j) - 1) / sizeof(uint64_j);

    int64_j StkPos      = -1;
    uint64_j* Stack     = (uint64_j*)SE->Malloc(SE, stklen);
    int key             = JSTKN_Unknown;
    void* Thing         = nullptr;

    while (cur < lst)
    {
        while ((cur < lst) && jsspace(*cur))
        {
            ++cur;
        }

        if (cur >= lst)
        {
            break;
        }

        switch (cur[0])
        {
        case 'n'    : result    = jstknParseNull(&Thing, &cur, lst, &StkPos, Stack, &key, SE);          break;
        case 't'    : result    = jstknParseTrue(&Thing, &cur, lst, &StkPos, Stack, &key, SE);          break;
        case 'f'    : result    = jstknParseFalse(&Thing, &cur, lst, &StkPos, Stack, &key, SE);         break;
        case '-'    :
        case '0'    :
        case '1'    :
        case '2'    :
        case '3'    :
        case '4'    :
        case '5'    :
        case '6'    :
        case '7'    :
        case '8'    :
        case '9'    : result    = jstknParseNumber(&Thing, &cur, lst, &StkPos, Stack, &key, SE);        break;
        case '"'    : result    = jstknParseString(&Thing, &cur, lst, &StkPos, Stack, &key, SE);        break;
        case '['    : result    = jstknParseArrayBegin(&Thing, &cur, lst, &StkPos, Stack, &key, SE);    break;
        case ']'    : result    = jstknParseArrayEnd(&Thing, &cur, lst, &StkPos, Stack, &key, SE);      break;
        case '{'    : result    = jstknParseObjectBegin(&Thing, &cur, lst, &StkPos, Stack, &key, SE);   break;
        case '}'    : result    = jstknParseObjectEnd(&Thing, &cur, lst, &StkPos, Stack, &key, SE);     break;
        case ':'    : result    = jstknParseColon(&Thing, &cur, lst, &StkPos, Stack, &key, SE);         break;
        case ','    : result    = jstknParseComma(&Thing, &cur, lst, &StkPos, Stack, &key, SE);         break;
        }

        if (!result)
        {
            break;
        }
    }

    SE->Free(SE, Stack, stklen);

    while ((cur < lst) && jsspace(cur[0]))
    {
        ++cur;
    }

    return (cur >= lst);
}

int jstknParseNull(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    const char* start   = *fst;
    *Thing              = SE->Begin(SE, JSTKN_Null, start);

    if ((*key != JSTKN_Unknown) && (*key != JSTKN_Any) && (*key != JSTKN_Null))
    {
        return SE->Fail(SE, *Thing, start);
    }

    if ((lst - start) < 4)
    {
        return SE->Fail(SE, *Thing, start);
    }

    if ((start[1] != 'u')   ||
        (start[2] != 'l')   ||
        (start[3] != 'l')   ||
        0)
    {
        return SE->Fail(SE, *Thing, start);
    }

    *fst    += 4;

    *key    = JSTKN_Unknown;

    SE->End(SE, *Thing, *fst);

    return 1;
}

int jstknParseTrue(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    const char* start   = *fst;
    *Thing          = SE->Begin(SE, JSTKN_True, start);

    if ((*key != JSTKN_Unknown) && (*key != JSTKN_Any) && (*key != JSTKN_True))
    {
        return SE->Fail(SE, *Thing, start);
    }

    if ((lst - start) < 4)
    {
        return SE->Fail(SE, *Thing, start);
    }

    if ((start[1] != 'r')   ||
        (start[2] != 'u')   ||
        (start[3] != 'e')   ||
        0)
    {
        return SE->Fail(SE, *Thing, start);
    }

    *fst    += 4;

    *key    = JSTKN_Unknown;

    SE->End(SE, *Thing, *fst);

    return 1;
}

int jstknParseFalse(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    const char* start   = *fst;
    *Thing          = SE->Begin(SE, JSTKN_False, start);

    if ((*key != JSTKN_Unknown) && (*key != JSTKN_Any) && (*key != JSTKN_False))
    {
        return SE->Fail(SE, *Thing, start);
    }

    if ((lst - start) < 5)
    {
        return SE->Fail(SE, *Thing, start);
    }

    if ((start[1] != 'a')   ||
        (start[2] != 'l')   ||
        (start[3] != 's')   ||
        (start[4] != 'e')   ||
        0)
    {
        return SE->Fail(SE, *Thing, start);
    }

    *fst    += 5;

    *key    = JSTKN_Unknown;

    SE->End(SE, *Thing, *fst);

    return 1;
}

int jstknParseNumber(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    const char* cur     = *fst;
    const char* before  = nullptr;
    *Thing              = SE->Begin(SE, JSTKN_Number, cur);

    if ((*key != JSTKN_Unknown) && (*key != JSTKN_Any) && (*key != JSTKN_Number))
    {
        return SE->Fail(SE, *Thing, cur);
    }

    if (cur[0] == '-')
    {
        ++cur;
    }

    if (cur >= lst)
    {
        return 0;
    }

    if (('1' <= cur[0]) && (cur[0] <= '9'))
    {
        while ((cur < lst) && (('0' <= cur[0]) && (cur[0] <= '9')))
        {
            ++cur;
        }
    }
    else if (cur[0] == '0')
    {
        ++cur;
    }

    if (cur >= lst)
    {
        goto DONE_NUMBER;
    }

    if (cur[0] == '.')
    {
        ++cur;

        if (cur >= lst)
        {
            return SE->Fail(SE, *Thing, cur);
        }

        before  = cur;

        while ((cur < lst) && (('0' <= cur[0]) && (cur[0] <= '9')))
        {
            ++cur;
        }

        if (before == cur)
        {
            return SE->Fail(SE, *Thing, cur);
        }
    }

    if (cur >= lst)
    {
        goto DONE_NUMBER;
    }

    if ((cur[0] != 'e') && (cur[0] != 'E'))
    {
        goto DONE_NUMBER;
    }

    ++cur;

    if (cur >= lst)
    {
        return SE->Fail(SE, *Thing, cur);
    }

    if ((cur[0] == '-') || (cur[0] == '+'))
    {
        ++cur;

        if (cur >= lst)
        {
            return SE->Fail(SE, *Thing, cur);
        }
    }

    before  = cur;

    while ((cur < lst) && (('0' <= cur[0]) && (cur[0] <= '9')))
    {
        ++cur;
    }

    if (before == cur)
    {
        return SE->Fail(SE, *Thing, cur);
    }

DONE_NUMBER:
    *fst            = cur;

    *key            = JSTKN_Unknown;

    SE->End(SE, *Thing, *fst);

    return 1;
}

int jstknParseString(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    const char *cur     = *fst;
    *Thing              = SE->Begin(SE, JSTKN_String, cur);

    if ((*key != JSTKN_Unknown) && (*key != JSTKN_Any) && (*key != JSTKN_String))
    {
        return SE->Fail(SE, *Thing, cur);
    }

    ++cur;
    // Restrict to ASCII.
    while (cur < lst)
    {
        if ((int)cur[0] >= (int)128)
        {
            return SE->Fail(SE, *Thing, cur);
        }
        if (cur[0] == '\\')
        {
            if ((cur+1) >= lst)
            {
                return SE->Fail(SE, *Thing, cur);
            }
            ++cur;
            switch (cur[0])
            {
            case '"'    :
            case '\\'   :
            case '/'    :
            case 'b'    :
            case 'f'    :
            case 'n'    :
            case 'r'    :
            case 't'    : ++cur; break;
            default     :
                if ((('0' <= cur[0]) && (cur[0] <= '9')) ||
                    (('a' <= cur[0]) && (cur[0] <= 'f')) ||
                    (('A' <= cur[0]) && (cur[0] <= 'F')) ||
                    0)
                {
                    if ((cur+4) >= lst)
                    {
                        return SE->Fail(SE, *Thing, cur);
                    }
                    for (int UU = 0; UU < 4; ++ UU)
                    {
                        if ((('0' <= cur[0]) && (cur[0] <= '9')) ||
                            (('a' <= cur[0]) && (cur[0] <= 'f')) ||
                            (('A' <= cur[0]) && (cur[0] <= 'F')) ||
                            0)
                        {
                            continue;
                        }
                        return SE->Fail(SE, *Thing, cur);
                    }
                    cur += 4;
                }
            }
        }
        else
        {
            if (cur[0] == '"')
            {
                break;
            }
            ++cur;
        }
    }
    if ((cur >= lst) || cur[0] != '"')
    {
        return SE->Fail(SE, *Thing, cur);
    }
    ++cur;

    *fst            = cur;

    *key            = JSTKN_Unknown;

    SE->End(SE, *Thing, *fst);

    return 1;
}

int jstknParseArrayBegin(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    *Thing          = SE->Begin(SE, JSTKN_Array, *fst);

    if ((*key != JSTKN_Unknown) && (*key != JSTKN_Any) && (*key != JSTKN_Array))
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    ++*fst;

    *StkPos         += 1;

    uint64_j STK    = Stack[*StkPos / sizeof(uint64_j)];

    STK             &= ~(0x1 << (*StkPos % sizeof(uint64_j)));

    Stack[*StkPos / sizeof(uint64_j)]    = STK;

    *key            = JSTKN_Unknown;

    return 1;
}

int jstknParseArrayEnd(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    if (*StkPos < 0)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    int RType   = (Stack[*StkPos / sizeof(uint64_j)] >> (*StkPos % sizeof(uint64_j))) & 0x1;

    if (RType != 0x0)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    if (*key != JSTKN_Unknown)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    --*StkPos;

    ++*fst;

    *key    = JSTKN_Unknown;

    SE->End(SE, *Thing, *fst);

    return 1;
}

int jstknParseObjectBegin(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    *Thing          = SE->Begin(SE, JSTKN_Object, *fst);

    if ((*key != JSTKN_Unknown) && (*key != JSTKN_Any) && (*key != JSTKN_Object))
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    ++*fst;

    *StkPos         += 1;

    uint64_j STK    = Stack[*StkPos / sizeof(uint64_j)];

    STK             |= (0x1 << (*StkPos % sizeof(uint64_j)));

    Stack[*StkPos / sizeof(uint64_j)]    = STK;

    *key            = JSTKN_Unknown;

    return 1;
}

int jstknParseObjectEnd(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    if (*StkPos < 0)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    int RType   = (Stack[*StkPos / sizeof(uint64_j)] >> (*StkPos % sizeof(uint64_j))) & 0x1;

    if (RType != 0x1)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    if (*key != JSTKN_Unknown)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    --*StkPos;

    ++*fst;

    *key    = JSTKN_Unknown;

    SE->End(SE, *Thing, *fst);

    return 1;
}

int jstknParseColon(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    if (*StkPos < 0)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    int RType   = (Stack[*StkPos / sizeof(uint64_j)] >> (*StkPos % sizeof(uint64_j))) & 0x1;

    if (RType != 0x1)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    if (*key != JSTKN_Unknown)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    *key        = JSTKN_Any;

    ++*fst;

    return 1;
}

int jstknParseComma(void** Thing, const char **fst, const char *lst, int64_j* StkPos, uint64_j* Stack, int* key, JSTKNSideEffect* SE)
{
    if (*StkPos < 0)
    {
        return SE->Fail(SE, *Thing, *fst);
    }

    int RType   = (Stack[*StkPos / sizeof(uint64_j)] >> (*StkPos % sizeof(uint64_j))) & 0x1;

    if (RType == 0x1)
    {
        if (*key != JSTKN_Unknown)
        {
            return SE->Fail(SE, *Thing, *fst);
        }
        *key    = JSTKN_String;
    }
    else
    {
        *key    = JSTKN_Any;
    }

    ++*fst;

    return 1;
}

#undef CHAR_BIT_J

#endif//JSTKN_C_SOURCE
