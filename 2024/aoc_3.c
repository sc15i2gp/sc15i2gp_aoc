#include "util.h"

int main()
{
    u32 in_file_size;
    char *in_file_contents = (char*)read_file_contents("aoc_3.txt", &in_file_size);

    tokeniser t = init_tokeniser(in_file_contents, in_file_size);

    u32 mul_sum = 0;
    char *start = in_file_contents;
    char *end   = start + in_file_size;
    token tok = {0};
    do
    {
        for(; *start != 'm' && start < end; start += 1);
        tokeniser t = init_tokeniser(start, in_file_size - (start - in_file_contents));
        start += 1;

        u32 op_0;
        u32 op_1;
        tok = read_token(&t);
        if(!str_eq(tok.loc, tok.len, "mul", 3)) continue;
        tok = read_token(&t);
        if(tok.type != TOKEN_OPAREN) continue;

        tok = read_token(&t);
        if(tok.type != TOKEN_INTEGER) continue;
        op_0 = tok.int_val;

        tok = read_token(&t);
        if(tok.type != TOKEN_COMMA) continue;

        tok = read_token(&t);
        if(tok.type != TOKEN_INTEGER) continue;
        op_1 = tok.int_val;

        tok = read_token(&t); 
        if(tok.type != TOKEN_CPAREN) continue;

        mul_sum += op_0 * op_1;
    }
    while(start < end && tok.type != TOKEN_END);

    printf("Part 1: Sum of mults = %u\n", mul_sum);

    u32 apply = 1;
    mul_sum = 0;
    start = in_file_contents;
    end   = start + in_file_size;
    do
    {
        for(; (*start != 'm' && *start != 'd') && start < end; start += 1);
        tokeniser t = init_tokeniser(start, in_file_size - (start - in_file_contents));
        tok = read_token(&t);
        if(*start == 'm' && apply)
        {
            start += 1;
            u32 op_0;
            u32 op_1;
            if(!str_eq(tok.loc, tok.len, "mul", 3)) continue;
            tok = read_token(&t);
            if(tok.type != TOKEN_OPAREN) continue;

            tok = read_token(&t);
            if(tok.type != TOKEN_INTEGER) continue;
            op_0 = tok.int_val;

            tok = read_token(&t);
            if(tok.type != TOKEN_COMMA) continue;

            tok = read_token(&t);
            if(tok.type != TOKEN_INTEGER) continue;
            op_1 = tok.int_val;

            tok = read_token(&t); 
            if(tok.type != TOKEN_CPAREN) continue;

            mul_sum += op_0 * op_1;
        }
        else if(*start == 'd')
        {
            start += 1;
            if(str_eq(tok.loc, tok.len, "do", 2))
            {
                tok = read_token(&t);
                if(tok.type != TOKEN_OPAREN) continue;
                tok = read_token(&t);
                if(tok.type != TOKEN_CPAREN) continue;
                apply = 1;
            }
            else if(str_eq(tok.loc, tok.len, "don", 3))
            {
                tok = read_token(&t);
                if(tok.type != TOKEN_APOST) continue;
                tok = read_token(&t);
                if(tok.type != TOKEN_WORD || tok.len != 1 || *tok.loc != 't') continue;
                tok = read_token(&t);
                if(tok.type != TOKEN_OPAREN) continue;
                tok = read_token(&t);
                if(tok.type != TOKEN_CPAREN) continue;
                apply = 0;
            }
        }
        else
        {
            start += 1;
            continue;
        }
    }
    while(start < end && tok.type != TOKEN_END);

    printf("Part 2: Sum of mults = %u\n", mul_sum);
    return 0;
}
