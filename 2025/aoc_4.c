#include "util.h"

char example[] =
{
    "..@@.@@@@.\n"
    "@@@.@.@.@@\n"
    "@@@@@.@.@@\n"
    "@.@@@@..@.\n"
    "@@.@@@@.@@\n"
    ".@@@@@@@.@\n"
    ".@.@.@.@@@\n"
    "@.@@@.@@@@\n"
    ".@@@@@@@@.\n"
    "@.@.@@@.@.\n"
};

typedef struct
{
    char *base;
    u32 w, h;   // Sizes including padding
} board;

char pad = '_';
char nil = '.';
void print_board(board *b)
{
    printf("Board: %lu x %lu\n", b->w, b->h);
    for(u32 i = 0, j = 0; i < b->h; i += 1, j += b->w)
    {
        for(u32 k = 0; k < b->w; k += 1)
        {
            char c = b->base[j+k];
            if(c < 10) c += '0';
            printf("%c", c);
        }
        printf("\n");
    }
}

char num_adjacent(board *b, u32 n)
{
    if(b->base[n] == pad || b->base[n] == nil) return -1;
    char num = 0;
    u32 t = n - b->w - 1;
    for(u32 i = 0; i < 3; i += 1, t += 1) if(b->base[t] != pad && b->base[t] != nil) num += 1;
    u32 l = n + b->w - 1;
    for(u32 i = 0; i < 3; i += 1, l += 1) if(b->base[l] != pad && b->base[l] != nil) num += 1;
    if(b->base[n-1] != pad && b->base[n-1] != nil) num += 1;
    if(b->base[n+1] != pad && b->base[n+1] != nil) num += 1;
    return num;
}

void count_adjacent(board *b)
{
    for(u32 i = b->w + 1; i < b->w*(b->h-1); i += 1)
    {
        if(b->base[i] == pad) i += 1;
        else
        { 
            char n = num_adjacent(b, i);
            if(n != -1) b->base[i] = n;
        }
    }
}

u32 all_lt_4(board *b)
{
    u32 n = 0;
    for(u32 i = b->w + 1; i < b->w*(b->h-1); i += 1)
    {
        if(b->base[i] == pad) i += 1;
        else if(b->base[i] < 4) n += 1;
    }
    return n;
}

u32 remove_lt_4(board *b)
{
    u32 n = 0;
    for(u32 i = b->w + 1; i < b->w*(b->h-1); i += 1)
    {
        if(b->base[i] == pad) i += 1;
        else if(b->base[i] < 4) { b->base[i] = '.'; n += 1; }
    }
    return n;
}

board read_input(char *in, u32 in_len)
{
    // Work out w and h then alloc
    u32 in_w = 0, in_h = 0;
    for(; in[in_w] != '\n'; in_w += 1);
    for(u32 i = 0; i < in_len; i += 1) if(in[i] == '\n') in_h += 1;
    u32 dst_w = in_w + 2, dst_h = in_h + 2;
    char *base = alloc(dst_w * dst_h);

    // Pad first row
    for(u32 i = 0; i < dst_w; i += 1) base[i] = pad;

    // Pad sides
    for(u32 i = 0, j = 0; i < dst_h; i += 1, j += 1)
    {
        base[j] = pad;
        j += dst_w-1;
        base[j] = pad;
    }
    
    // Pad last row
    for(u32 i = 0, j = dst_w*(dst_h-1); i < dst_w; i += 1, j += 1) base[j] = pad;

    // Fill board
    for(u32 src = 0, dst = dst_w+1; src < in_h*(in_w+1); src += 1, dst += 1)
    {
        if(in[src] == '\n') dst += 1;
        else base[dst] = in[src];
    }
    board b = {.w = dst_w, .h = dst_h, .base = base};
    return b;
}

int main()
{
#if 0
    const char *in = example;
    u32 in_len = strlen(in);
#else
    u32 in_len;
    char *in = read_file_contents("aoc_4.txt", &in_len);
#endif
    board b = read_input(in, in_len);
    count_adjacent(&b);
    u32 n = all_lt_4(&b);
    printf("Num < 4 = %lu\n", n);
    u32 total_removed = 0;
    u32 removed;
    do
    {
        removed = remove_lt_4(&b);
        count_adjacent(&b);
        total_removed += removed;
    }
    while(removed > 0);
    printf("Removed = %lu\n", total_removed);
    printf("DONE!\n");
    return 0;
}
