#include "util.h"

char example[] =
{
    "L68\n"
    "L30\n"
    "R48\n"
    "L5\n"
    "R60\n"
    "L55\n"
    "L1\n"
    "L99\n"
    "R14\n"
    "L82\n"
};

typedef struct
{
    u32 num_entries;
    u8  *dir; // 0 = L, 1 = R
    u32 *dist;
} dial_list;

void print_dial_list(dial_list *l)
{
    for(u32 i = 0; i < l->num_entries; i += 1)
    {
        if(l->dir[i] == 0) printf("L");
        else               printf("R");
        printf("%d\n", l->dist[i]);
    }
}

s32 turn_right(s32 pos, s32 dist, u32 *pass)
{
    *pass += (dist / 100);
    dist = dist % 100;
    pos += dist;
    if(pos > 99) { *pass += 1; pos = pos - 100; }
    return pos;
}

s32 turn_left(s32 pos, s32 dist, u32 *pass)
{
    u8 on_zero = pos == 0;
    *pass += (dist / 100);
    dist = dist % 100;
    pos -= dist;
    if(pos < 0) { if(!on_zero) *pass += 1; pos = 100 + pos; }
    if(pos == 0) *pass += 1;
    return pos;
}

u32 find_password(dial_list *l, s32 pos)
{
    u32 pass = 0;
    for(u32 i = 0; i < l->num_entries; i += 1)
    {
        s32 dist = (s32)l->dist[i];
        u8  dir = l->dir[i];
        u32 p;
        if(dir) pos = turn_right(pos, dist, &p);
        else    pos = turn_left(pos, dist, &p);
        if(pos == 0) pass += 1;
    }
    return pass;
}

u32 find_clk_password(dial_list *l, s32 pos)
{
    u32 pass = 0;
    for(u32 i = 0; i < l->num_entries; i += 1)
    {
        s32 dist = (s32)l->dist[i];
        u8  dir = l->dir[i];
        u32 p = 0;
        if(dir) pos = turn_right(pos, dist, &p);
        else    pos = turn_left(pos, dist, &p);
        pass += p;
    }
    return pass;
}

dial_list read_input(const char *in, u32 in_len)
{
    u32 num_entries = 0;
    tokeniser to = init_tokeniser(in, in_len);
    for(token t = read_token(&to); t.type != TOKEN_END; t = read_token(&to))
    {
        if(t.type == TOKEN_NEWLINE) num_entries += 1;
    }
    u8  *dir  = alloc(num_entries * sizeof(u8));
    u32 *dist = alloc(num_entries * sizeof(u32));
    to = init_tokeniser(in, in_len);
    for(u32 i = 0; i < num_entries; i += 1)
    {
        token t = read_token(&to);
        dir[i] = (t.loc[0] == 'L') ? 0 : 1;
        t = read_token(&to);
        dist[i] = (u32)(t.int_val);
        t = read_token(&to);
    }
    dial_list l = {.num_entries = num_entries, .dir = dir, .dist = dist};
    return l;
}

int main()
{
#if 0
    const char *in = example;
    u32 in_len = strlen(in);
#else
    u32 in_len;
    const char *in = read_file_contents("aoc_1.txt", &in_len);
#endif
    dial_list l = read_input(in, in_len);
    u32 password = find_password(&l, 50);
    printf("PASSWORD = %u\n", password);
    u32 clk_pws = find_clk_password(&l, 50);
    printf("PASSWORD = %u\n", clk_pws);
    printf("DONE!\n");
    return 0;
}
