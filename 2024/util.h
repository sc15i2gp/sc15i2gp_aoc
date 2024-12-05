#include <stdio.h>
#include <stdint.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;

void *alloc(u32 size)
{
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

u8 *read_file_contents(const char *path, u32 *ret_file_size)
{
    HANDLE file_handle   = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD  file_size     = GetFileSize(file_handle, NULL);
    u8    *file_contents = (u8*)alloc(file_size);
    DWORD  bytes_read;
    ReadFile(file_handle, file_contents, file_size, &bytes_read, NULL);
    CloseHandle(file_handle);
    
    *ret_file_size = (u32)file_size;
    return file_contents;
}

void dealloc(void *ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

typedef struct
{
    char *start;
    char *end;
    char *current;
} tokeniser;

tokeniser init_tokeniser(char *in, u32 in_len)
{
    tokeniser t;
    t.start   = in;
    t.end     = in + in_len;
    t.current = t.start;
    
    return t;
}

typedef enum
{
    TOKEN_NONE = 0,
    TOKEN_INTEGER,
    TOKEN_WORD,
    TOKEN_SPACE,
    TOKEN_NEWLINE = '\n',
    TOKEN_OPAREN = '(',
    TOKEN_CPAREN = ')',
    TOKEN_COMMA  = ',',
    TOKEN_APOST  = '\'',
    TOKEN_PIPE   = '|',
    TOKEN_END,
    TOKEN_COUNT
} token_type;

typedef struct
{
    char      *loc;
    u32        len;
    token_type type;

    u32 int_val;
} token;

u8 str_eq(const char *s0, u32 s0_len, const char *s1, u32 s1_len)
{
    if(s0_len != s1_len) return 0;
    for(u32 i = 0; i < s0_len; i += 1) if(s0[i] != s1[i]) return 0;

    return 1;
}

u8 is_num_char(char c)
{
    return (c >= '0') && (c <= '9');
}

u8 is_alpha_char(unsigned char c)
{
    c &= 0b11011111;
    c -= 'A';

    return c < 26;
}

u8 is_newline(char c)
{
    return c == '\n';
}

u8 is_space(char c)
{
    return c == ' ';
}

u32 str_to_int(char *str, u32 str_len)
{
    u32 int_val = 0;
    u32 radix   = 1;

    for(char *s = (str + str_len - 1); s >= str; s -= 1)
    {
        int_val += radix * (u32)(*s - '0');
        radix *= 10;
    }

    return int_val;
}

token read_token(tokeniser *t)
{
    token ret = {0};
    if(t->current >= t->end)
    {
        ret.type   = TOKEN_END;
        ret.loc    = t->end;
        ret.len    = 0;
        t->current = t->end;

        return ret;
    }

    if(is_num_char(*t->current))
    {
        ret.loc  = t->current;
        ret.type = TOKEN_INTEGER;
        for(; is_num_char(*t->current) && (t->current != t->end); t->current += 1);
        ret.len = t->current - ret.loc;
        ret.int_val = str_to_int(ret.loc, ret.len);

        return ret;
    }

    if(is_alpha_char(*t->current))
    {
        ret.loc  = t->current;
        ret.type = TOKEN_WORD;
        for(; is_alpha_char(*t->current) && (t->current != t->end); t->current += 1);
        ret.len = t->current - ret.loc;

        return ret;
    }

    if(is_space(*t->current))
    {
        ret.type = TOKEN_SPACE;
        ret.loc  = t->current;

        for(; is_space(*t->current) && (t->current != t->end); t->current += 1);
        ret.len = t->current - ret.loc;

        return ret;
    }

    switch(*t->current)
    {
        case TOKEN_OPAREN:
        case TOKEN_CPAREN:
        case TOKEN_COMMA:
        case TOKEN_APOST:
        case TOKEN_PIPE:
        case TOKEN_NEWLINE:
        {
            ret.type = *t->current;
            ret.loc  = t->current;
            ret.len  = 1;

            t->current += 1;

            return ret;
        }
    }

    ret.type = TOKEN_NONE;
    ret.loc  = NULL;
    ret.len  = 0;

    return ret;
}

void print_token(token *t)
{
    switch(t->type)
    {
        case TOKEN_OPAREN:
        case TOKEN_CPAREN:
        case TOKEN_COMMA:
        case TOKEN_APOST:
        case TOKEN_PIPE:
        {
            printf("TOKEN: Type(%c) Len:(%u)", t->type, t->len);
            break;
        }
        case TOKEN_INTEGER:
        {
            printf("TOKEN: Type:(INTEGER) Len:(%u) Str:(%.*s) Val:(%u)", t->len, t->len, t->loc, t->int_val);
            break;
        }
        case TOKEN_SPACE:
        case TOKEN_NEWLINE:
        {
            printf("TOKEN: Type:(WHITESPACE) Len:(%u)", t->len);
            break;
        }
        case TOKEN_END:
        {
            printf("TOKEN: Type:(END)");
            break;
        }
        case TOKEN_WORD:
        {
            printf("TOKEN: Type:(WORD) Len:(%u) Str(%.*s)", t->len, t->len, t->loc);
            break;
        }
        default:
        {
            printf("TOKEN UNKNOWN: Type:(%u) Len(%u)", t->type, t->len);
            break;
        }
    }
}
