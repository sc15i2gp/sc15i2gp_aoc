#include "util.h"

u32 count_num_pairs(tokeniser *t)
{
    u32 num_ints = 0;
    for(token tok = read_token(t); tok.type != TOKEN_END; tok = read_token(t))
    {
        if(tok.type == TOKEN_INTEGER) num_ints += 1;
    }
    return num_ints / 2;
}

void parse_pairs(tokeniser *t, u32 num_pairs, u32 *left, u32 *right)
{
   token tok;
   for(u32 i = 0; i < num_pairs; i += 1)
   {//No type checking in this loop - do not expect incorrect input
        tok = read_token(t); //INTEGER
        left[i] = tok.int_val;

        tok = read_token(t); //WHITESPACE
        tok = read_token(t); //INTEGER
        right[i] = tok.int_val;

        tok = read_token(t); //WHITESPACE
   } 
}

void bubble_sort(u32 *list, u32 len)
{
    u32 num_swaps;
    do
    {
        num_swaps = 0;
        for(u32 i = 0; i < len - 1; i += 1)
        {
            if(list[i] > list[i+1])
            {
                u32 tmp   = list[i];
                list[i]   = list[i+1];
                list[i+1] = tmp;
                num_swaps += 1;
            }
        }
    }
    while(num_swaps != 0);
}

int main()
{
    u32 in_file_size;
    u8 *in_file_contents = read_file_contents("aoc_1.txt", &in_file_size);

    tokeniser t = init_tokeniser(in_file_contents, in_file_size);

    u32 num_pairs = count_num_pairs(&t);
    t = init_tokeniser(in_file_contents, in_file_size);

    printf("Num pairs = %u\n", num_pairs);
    u32 *left  = (u32*)alloc(num_pairs * sizeof(u32));
    u32 *right = (u32*)alloc(num_pairs * sizeof(u32));

    parse_pairs(&t, num_pairs, left, right);

    bubble_sort(left, num_pairs);
    bubble_sort(right, num_pairs);
    
    u32 total_dist = 0;
    for(u32 i = 0; i < num_pairs; i += 1)
    {
        u32 l = left[i];
        u32 r = right[i];
        if(l > r) total_dist += l - r;
        else      total_dist += r - l;
    }

    printf("Part 1: Total dist = %u\n", total_dist);

    //n^2 loop bad but quick to write
    u32 sim_score = 0;
    for(u32 i = 0; i < num_pairs; i += 1)
    {
        u32 n = 0;
        u32 l = left[i];
        for(u32 j = 0; j < num_pairs; j += 1)
        {
            u32 r = right[j];
            if(r == l) n += 1;
        }
        sim_score += n * l;
    }

    printf("Part 2: Total sim score = %u\n", sim_score);
    return 0;
}
