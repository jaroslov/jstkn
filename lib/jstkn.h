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
    JSTKN_Bool,
    JSTKN_Number,
    JSTKN_String,
    JSTKN_Array,
    JSTKN_Object,
} JSTKNTypes;

typedef struct JSTKNSideEffect
{
    void*           Handle;
    void*         (*Begin)(JSTKNSideEffect*, JSTKNTypes, const char* ptr);
    int           (*End)(JSTKNSideEffect*, void*, const char* ptr);
    int           (*Fail)(JSTKNSideEffect*, void*, const char* ptr);

    void*         (*Malloc)(JSTKNSideEffect*, unsigned long long);
    void*         (*Realloc)(JSTKNSideEffect*, void*, unsigned long long);
    int           (*Free)(JSTKNSideEffect*, void*, unsigned long long);

    int           (*Schema)(JSTKNSideEffect*, JSTKNTypes, void*);
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
#define STACK_SCALE_J       ((CHAR_BIT_J) * sizeof(uint64_j))

typedef struct JSTKNContext
{
    void*               Thing;
    const char*         fst;
    const char*         lst;
    long long           StkPos;
    long long           StkLength;  // In contexts (1b)
    unsigned long long* Stack;
    int                 key;
    JSTKNSideEffect*    SE;
} JSTKNContext;

static inline int jstknParseNull(JSTKNContext* Jxt);
static inline int jstknParseTrue(JSTKNContext* Jxt);
static inline int jstknParseFalse(JSTKNContext* Jxt);
static inline int jstknParseNumber(JSTKNContext* Jxt);
static inline int jstknParseString(JSTKNContext* Jxt);
static inline int jstknParseArrayBegin(JSTKNContext* Jxt);
static inline int jstknParseArrayEnd(JSTKNContext* Jxt);
static inline int jstknParseObjectBegin(JSTKNContext* Jxt);
static inline int jstknParseObjectEnd(JSTKNContext* Jxt);
static inline int jstknParseColon(JSTKNContext* Jxt);
static inline int jstknParseComma(JSTKNContext* Jxt);

static inline int jsspace(char c)
{
    return !!((1ULL << c) & ((1ULL << 0x09) | (1ULL << 0x0A) | (1ULL << 0x0B) | (1ULL << 0x0C) | (1ULL << 0x0D) | (1ULL << 0x20)));
}

int jstknParse(const char* fst, const char* lst, JSTKNSideEffect* SE)
{
    JSTKNContext Jxt    =
    {
        .Thing          = nullptr,
        .fst            = fst,
        .lst            = lst,
        .StkPos         = -1,
        .StkLength      = 8 * sizeof(uint64_j) * CHAR_BIT_J,
        .Stack          = nullptr,
        .SE             = SE,
    };

    int result          = 0;

    Jxt.Stack           = (uint64_j*)SE->Malloc(SE, Jxt.StkLength / CHAR_BIT_J);
    Jxt.key             = JSTKN_Unknown;

    while (Jxt.fst < lst)
    {
        while ((Jxt.fst < lst) && jsspace(*Jxt.fst))
        {
            ++Jxt.fst;
        }

        if (Jxt.fst >= lst)
        {
            break;
        }

        switch (Jxt.fst[0])
        {
        case 'n'    : result    = jstknParseNull(&Jxt);         break;
        case 't'    : result    = jstknParseTrue(&Jxt);         break;
        case 'f'    : result    = jstknParseFalse(&Jxt);        break;
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
        case '9'    : result    = jstknParseNumber(&Jxt);       break;
        case '"'    : result    = jstknParseString(&Jxt);       break;
        case '['    : result    = jstknParseArrayBegin(&Jxt);   break;
        case ']'    : result    = jstknParseArrayEnd(&Jxt);     break;
        case '{'    : result    = jstknParseObjectBegin(&Jxt);  break;
        case '}'    : result    = jstknParseObjectEnd(&Jxt);    break;
        case ':'    : result    = jstknParseColon(&Jxt);        break;
        case ','    : result    = jstknParseComma(&Jxt);        break;
        }

        if (!result)
        {
            break;
        }
    }

    SE->Free(SE, Jxt.Stack, Jxt.StkLength);

    while ((Jxt.fst < lst) && jsspace(Jxt.fst[0]))
    {
        ++Jxt.fst;
    }

    return (Jxt.fst >= lst);
}

int jstknParseNull(JSTKNContext* Jxt)
{
    const char* start   = Jxt->fst;
    Jxt->Thing          = Jxt->SE->Begin(Jxt->SE, JSTKN_Null, start);

    if (!Jxt->SE->Schema(Jxt->SE, JSTKN_Null, Jxt->Thing))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((Jxt->key != JSTKN_Unknown) && (Jxt->key != JSTKN_Any) && (Jxt->key != JSTKN_Null))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((Jxt->lst - start) < 4)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((start[1] != 'u')   ||
        (start[2] != 'l')   ||
        (start[3] != 'l')   ||
        0)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    Jxt->fst    += 4;

    Jxt->key    = JSTKN_Unknown;

    Jxt->SE->End(Jxt->SE, Jxt->Thing, Jxt->fst);

    return 1;
}

int jstknParseTrue(JSTKNContext* Jxt)
{
    const char* start   = Jxt->fst;
    Jxt->Thing          = Jxt->SE->Begin(Jxt->SE, JSTKN_True, start);

    if (!Jxt->SE->Schema(Jxt->SE, JSTKN_Bool, Jxt->Thing))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((Jxt->key != JSTKN_Unknown) && (Jxt->key != JSTKN_Any) && (Jxt->key != JSTKN_True))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((Jxt->lst - start) < 4)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((start[1] != 'r')   ||
        (start[2] != 'u')   ||
        (start[3] != 'e')   ||
        0)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    Jxt->fst    += 4;

    Jxt->key    = JSTKN_Unknown;

    Jxt->SE->End(Jxt->SE, Jxt->Thing, Jxt->fst);

    return 1;
}

int jstknParseFalse(JSTKNContext* Jxt)
{
    const char* start   = Jxt->fst;
    Jxt->Thing          = Jxt->SE->Begin(Jxt->SE, JSTKN_False, start);

    if (!Jxt->SE->Schema(Jxt->SE, JSTKN_Bool, Jxt->Thing))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((Jxt->key != JSTKN_Unknown) && (Jxt->key != JSTKN_Any) && (Jxt->key != JSTKN_False))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((Jxt->lst - start) < 5)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    if ((start[1] != 'a')   ||
        (start[2] != 'l')   ||
        (start[3] != 's')   ||
        (start[4] != 'e')   ||
        0)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, start);
    }

    Jxt->fst    += 5;

    Jxt->key    = JSTKN_Unknown;

    Jxt->SE->End(Jxt->SE, Jxt->Thing, Jxt->fst);

    return 1;
}

int jstknParseNumber(JSTKNContext* Jxt)
{
    const char* cur     = Jxt->fst;
    const char* before  = nullptr;
    Jxt->Thing          = Jxt->SE->Begin(Jxt->SE, JSTKN_Number, cur);

    if (!Jxt->SE->Schema(Jxt->SE, JSTKN_Number, Jxt->Thing))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
    }

    if ((Jxt->key != JSTKN_Unknown) && (Jxt->key != JSTKN_Any) && (Jxt->key != JSTKN_Number))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
    }

    if (cur[0] == '-')
    {
        ++cur;
    }

    if (cur >= Jxt->lst)
    {
        return 0;
    }

    if (('1' <= cur[0]) && (cur[0] <= '9'))
    {
        while ((cur < Jxt->lst) && (('0' <= cur[0]) && (cur[0] <= '9')))
        {
            ++cur;
        }
    }
    else if (cur[0] == '0')
    {
        ++cur;
    }

    if (cur >= Jxt->lst)
    {
        goto DONE_NUMBER;
    }

    if (cur[0] == '.')
    {
        ++cur;

        if (cur >= Jxt->lst)
        {
            return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
        }

        before  = cur;

        while ((cur < Jxt->lst) && (('0' <= cur[0]) && (cur[0] <= '9')))
        {
            ++cur;
        }

        if (before == cur)
        {
            return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
        }
    }

    if (cur >= Jxt->lst)
    {
        goto DONE_NUMBER;
    }

    if ((cur[0] != 'e') && (cur[0] != 'E'))
    {
        goto DONE_NUMBER;
    }

    ++cur;

    if (cur >= Jxt->lst)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
    }

    if ((cur[0] == '-') || (cur[0] == '+'))
    {
        ++cur;

        if (cur >= Jxt->lst)
        {
            return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
        }
    }

    before  = cur;

    while ((cur < Jxt->lst) && (('0' <= cur[0]) && (cur[0] <= '9')))
    {
        ++cur;
    }

    if (before == cur)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
    }

DONE_NUMBER:
    Jxt->fst        = cur;

    Jxt->key        = JSTKN_Unknown;

    Jxt->SE->End(Jxt->SE, Jxt->Thing, Jxt->fst);

    return 1;
}

int jstknParseString(JSTKNContext* Jxt)
{
    const char *cur     = Jxt->fst;
    Jxt->Thing          = Jxt->SE->Begin(Jxt->SE, JSTKN_String, cur);

    if (!Jxt->SE->Schema(Jxt->SE, JSTKN_String, Jxt->Thing))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
    }

    if ((Jxt->key != JSTKN_Unknown) && (Jxt->key != JSTKN_Any) && (Jxt->key != JSTKN_String))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
    }

    ++cur;
    // Restrict to ASCII.
    while (cur < Jxt->lst)
    {
        if ((unsigned)cur[0] >= (unsigned)128)
        {
            // Handle super-ASCII (UTF-8), here.
            switch ((unsigned)cur[0])
            {
            case 0xC0   : // Invalid encodings; reject.
            case 0xC1   :
            case 0xF5   :
            case 0xF6   :
            case 0xF7   :
            case 0xF8   :
            case 0xF9   :
            case 0xFA   :
            case 0xFB   :
            case 0xFC   :
            case 0xFD   :
            case 0xFE   :
            case 0xFF   : return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
            }
            unsigned long encoding  = 0;
            if (0xF0 == ((unsigned char)cur[0] & ~0x7))
            {
                if ((Jxt->lst - cur) <= 4)
                {
                    return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                if ((0x80 != ((unsigned char)cur[1] & ~0x3F))  ||
                    (0x80 != ((unsigned char)cur[2] & ~0x3F))  ||
                    (0x80 != ((unsigned char)cur[3] & ~0x3F))  ||
                    0)
                {
                    return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                encoding            = ((unsigned char)cur[0] & 0x07) << 18
                                    | ((unsigned char)cur[1] & 0x3F) << 12
                                    | ((unsigned char)cur[2] & 0x3F) << 6
                                    | ((unsigned char)cur[3] & 0x3F) << 0
                                    ;
                switch ((unsigned char)cur[0])
                {
                case 0xF0   : if (encoding < 0x10000) return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                case 0xF1   :
                case 0xF2   :
                case 0xF3   : if (encoding < 0x40000) return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                case 0xF4   : if (encoding < 0x100000) return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                default     : return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                cur += 4;
            }
            else if (0xE0 == ((unsigned char)cur[0] & ~0xF))
            {
                if ((Jxt->lst - cur) <= 3)
                {
                    return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                if ((0x80 != ((unsigned char)cur[1] & ~0x3F))  ||
                    (0x80 != ((unsigned char)cur[2] & ~0x3F))  ||
                    0)
                {
                    return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                encoding            = ((unsigned char)cur[0] & 0x0F) << 12
                                    | ((unsigned char)cur[1] & 0x3F) << 6
                                    | ((unsigned char)cur[2] & 0x3F) << 0
                                    ;
                switch ((unsigned char)cur[0])
                {
                case 0xE0   : if (encoding < 0x800) return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                default     : if (encoding < 0x1000) return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                cur += 3;
            }
            else if (0xC0 == ((unsigned char)cur[0] & ~0x1F))
            {
                if ((Jxt->lst - cur) <= 2)
                {
                    return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                if ((0x80 != ((unsigned char)cur[1] & ~0x3F))  ||
                    0)
                {
                    return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                encoding            = ((unsigned char)cur[0] & 0x1F) << 6
                                    | ((unsigned char)cur[1] & 0x3F) << 0
                                    ;
                if (encoding < 0x80)
                {
                    return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
                }
                cur += 2;
            }
            else
            {
                return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
            }
        }
        else if (cur[0] == '\\')
        {
            if ((cur+1) >= Jxt->lst)
            {
                return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
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
                    if ((cur+4) >= Jxt->lst)
                    {
                        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
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
                        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
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
    if ((cur >= Jxt->lst) || cur[0] != '"')
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, cur);
    }
    ++cur;

    Jxt->fst        = cur;

    Jxt->key        = JSTKN_Unknown;

    Jxt->SE->End(Jxt->SE, Jxt->Thing, Jxt->fst);

    return 1;
}

int jstknParseArrayBegin(JSTKNContext* Jxt)
{
    Jxt->Thing      = Jxt->SE->Begin(Jxt->SE, JSTKN_Array, Jxt->fst);

    if (!Jxt->SE->Schema(Jxt->SE, JSTKN_Array, Jxt->Thing))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    if ((Jxt->key != JSTKN_Unknown) && (Jxt->key != JSTKN_Any) && (Jxt->key != JSTKN_Array))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    ++Jxt->fst;

    Jxt->StkPos     += 1;

    if (Jxt->StkPos >= Jxt->StkLength)
    {
        Jxt->StkLength  *= 2;
        Jxt->Stack      = (unsigned long long*)Jxt->SE->Realloc(Jxt->SE, Jxt->Stack, Jxt->StkLength / CHAR_BIT_J);
        if (Jxt->Stack == nullptr)
        {
            return 0;
        }
    }

    uint64_j STK    = Jxt->Stack[Jxt->StkPos / STACK_SCALE_J];

    STK             &= ~(0x1 << (Jxt->StkPos % STACK_SCALE_J));

    Jxt->Stack[Jxt->StkPos / STACK_SCALE_J]    = STK;

    Jxt->key        = JSTKN_Unknown;

    return 1;
}

int jstknParseArrayEnd(JSTKNContext* Jxt)
{
    if (Jxt->StkPos < 0)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    int RType   = (Jxt->Stack[Jxt->StkPos / STACK_SCALE_J] >> (Jxt->StkPos % STACK_SCALE_J)) & 0x1;

    if (RType != 0x0)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    if (Jxt->key != JSTKN_Unknown)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    --Jxt->StkPos;

    ++Jxt->fst;

    Jxt->key        = JSTKN_Unknown;

    Jxt->SE->End(Jxt->SE, Jxt->Thing, Jxt->fst);

    return 1;
}

int jstknParseObjectBegin(JSTKNContext* Jxt)
{
    Jxt->Thing      = Jxt->SE->Begin(Jxt->SE, JSTKN_Object, Jxt->fst);

    if (!Jxt->SE->Schema(Jxt->SE, JSTKN_Object, Jxt->Thing))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    if ((Jxt->key != JSTKN_Unknown) && (Jxt->key != JSTKN_Any) && (Jxt->key != JSTKN_Object))
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    ++Jxt->fst;

    Jxt->StkPos     += 1;

    if (Jxt->StkPos >= Jxt->StkLength)
    {
        Jxt->StkLength  *= 2;
        Jxt->Stack      = (unsigned long long*)Jxt->SE->Realloc(Jxt->SE, Jxt->Stack, Jxt->StkLength / CHAR_BIT_J);
        if (Jxt->Stack == nullptr)
        {
            return 0;
        }
    }

    uint64_j STK    = Jxt->Stack[Jxt->StkPos / STACK_SCALE_J];

    STK             |= (0x1 << (Jxt->StkPos % STACK_SCALE_J));

    Jxt->Stack[Jxt->StkPos / STACK_SCALE_J]    = STK;

    Jxt->key        = JSTKN_Unknown;

    return 1;
}

int jstknParseObjectEnd(JSTKNContext* Jxt)
{
    if (Jxt->StkPos < 0)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    int RType   = (Jxt->Stack[Jxt->StkPos / STACK_SCALE_J] >> (Jxt->StkPos % STACK_SCALE_J)) & 0x1;

    if (RType != 0x1)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    if (Jxt->key != JSTKN_Unknown)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    --Jxt->StkPos;

    ++Jxt->fst;

    Jxt->key        = JSTKN_Unknown;

    Jxt->SE->End(Jxt->SE, Jxt->Thing, Jxt->fst);

    return 1;
}

int jstknParseColon(JSTKNContext* Jxt)
{
    if (Jxt->StkPos < 0)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    int RType   = (Jxt->Stack[Jxt->StkPos / STACK_SCALE_J] >> (Jxt->StkPos % STACK_SCALE_J)) & 0x1;

    if (RType != 0x1)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    if (Jxt->key != JSTKN_Unknown)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    Jxt->key    = JSTKN_Any;

    ++Jxt->fst;

    return 1;
}

int jstknParseComma(JSTKNContext* Jxt)
{
    if (Jxt->StkPos < 0)
    {
        return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
    }

    int RType   = (Jxt->Stack[Jxt->StkPos / STACK_SCALE_J] >> (Jxt->StkPos % STACK_SCALE_J)) & 0x1;

    if (RType == 0x1)
    {
        if (Jxt->key != JSTKN_Unknown)
        {
            return Jxt->SE->Fail(Jxt->SE, Jxt->Thing, Jxt->fst);
        }
        Jxt->key    = JSTKN_String;
    }
    else
    {
        Jxt->key    = JSTKN_Any;
    }

    ++Jxt->fst;

    return 1;
}

#undef CHAR_BIT_J

#endif//JSTKN_C_SOURCE
