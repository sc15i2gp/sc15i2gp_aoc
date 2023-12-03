#include <stdio.h>
#include <stdint.h>
#include <windows.h>

typedef uint32_t u32;

const char in_arg[] =
#if 0
"467..114..\n"
"...*......\n"
"..35..633.\n"
"......#...\n"
"617*......\n"
".....+.58.\n"
"..592.....\n"
"......755.\n"
"...$.*....\n"
".664.598..\n";
#else
#include "aoc_2023_3.txt"
#endif

u32 is_number(const char *c)
{
    return '0' <= *c && *c <= '9';
}

u32 read_number_and_overwrite(char *c)
{
    char *num_start = c;
    char *num_end   = c;

    for(; is_number(num_start); --num_start);
    ++num_start;

    for(; is_number(num_end); ++num_end);
    --num_end;

    u32 n    = 0;
    u32 base = 1;

    for(c = num_end; c >= num_start; --c)
    {
        u32 cv = *c - '0';
        n += base * cv;
        base *= 10;

        *c = '.';
    }

    return n;
}

//Assuming that there are no symbols on the first or last lines of in_arg
//(which tracks with the given input)
int main()
{
    u32 in_width = 1; //Start at 1 since in_arg contains '\n'
    u32 in_lines = 0;
    for(const char *c = in_arg; *c != '\n'; ++c, ++in_width);
    for(const char *c = in_arg; *c != 0; ++c)
    {
        if(*c == '\n') ++in_lines;
    }
    printf("Width: %u Lines %u\n", in_width, in_lines);

    u32 in_size = sizeof(in_arg) - 1;
    
    char *in = VirtualAlloc(NULL, in_size + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    in[0] = 0;
    in += 1; //Pad in with 0
    memcpy(in, in_arg, in_size);
    printf("IN: %p\n", in);

    u32 sum      = 0;
    u32 gear_sum = 0;
    char *in_end = in + in_size;
    for(char *c = in; c < in_end; ++c)
    {
        if(!is_number(c) && *c != '.' && *c != '\n')
        {//Symbol found
            //Look at adjacent above
            char *adj = (c - (in_width+1));
            u32 num_parts   = 0;
            u32 parts_sum   = 0;
            u32 parts_ratio = 1;
            for(u32 row = 0; row < 3; ++row)
            {
                for(u32 col = 0; col < 3; ++col)
                {
                    if(is_number(&adj[col]))
                    {
                        u32 n = read_number_and_overwrite(&adj[col]);
                        parts_sum   += n;
                        parts_ratio *= n;
                        ++num_parts;
                        printf("ADJ: %u\n", n);
                    }

                }
                adj += in_width;
            }
            if(*c == '*' && num_parts == 2) 
            {
                gear_sum += parts_ratio;
            }
            sum += parts_sum;
        }
    }

    printf("SUM: %u\n", sum);
    printf("GEAR SUM: %u\n", gear_sum);
    return 0;
}
