#include <stdio.h>
#include <stdint.h>

typedef uint32_t u32;

const char in_arg[] =
#if 0
"Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green\n"
"Game 2: 1 blue, 2 green; 3 green, 4 blue, 1 red; 1 green, 1 blue\n"
"Game 3: 8 green, 6 blue, 20 red; 5 blue, 4 red, 13 green; 5 green, 1 red\n"
"Game 4: 1 green, 3 red, 6 blue; 3 green, 6 red; 3 green, 15 blue, 14 red\n"
"Game 5: 6 red, 1 blue, 3 green; 2 blue, 1 red, 2 green\n";
#else
#include "aoc_2023_2.txt"
#endif

//For simplicity, assume str_1 is a single NULL terminated string
//but str_0 may be much larger (i.e. input length)
u32 str_cmp(const char *str_0, const char *str_1)
{
    for(u32 i = 0; str_1[i] != 0; ++i)
    {
        if(str_0[i] != str_1[i]) return 0;
    }
    return 1;
}

u32 is_num(const char *str)
{
    return '0' <= *str && *str <= '9';
}

u32 str_to_num(const char *str)
{
    const char *str_end = str;
    for(; is_num(str_end); ++str_end);
    --str_end;

    u32 num = 0;
    u32 base = 1;
    for(; str_end >= str; --str_end)
    {
        u32 digit = *str_end - '0';
        num       += digit * base;
        base      *= 10;
    }

    return num;
}

const char *find_next_char(const char *str, char c)
{
    for(; *str != c; ++str);
    return str;
}

const char *find_next_space(const char *str)
{
    return find_next_char(str, ' ');
}

const char *find_next_non_space(const char *str)
{
    for(; *str == ' '; ++str);
    return str;
}

const char *find_next_non_letter(const char *str)
{
    for(; 'a' <= *str && *str <= 'z'; ++str);
    return str;
}

int main()
{
    printf("STARTING\n");

    u32 in_arg_length = sizeof(in_arg) - 1;
    const char *in_arg_end = in_arg + in_arg_length;

    u32 game_sum  = 0;
    u32 power_sum = 0;
    u32 red_max   = 12;
    u32 green_max = 13;
    u32 blue_max  = 14;
    for(const char *c = in_arg; c < in_arg_end; ++c)
    {
        //Skip "Game " and get to game id
        c = find_next_space(c);
        c = find_next_non_space(c);

        u32 game_id       = str_to_num(c);
        u32 is_game_valid = 1;
        u32 red_drawn     = 0;
        u32 green_drawn   = 0;
        u32 blue_drawn    = 0;
        printf("Game %u: ", game_id);

        c = find_next_char(c, ':');
        for(; *c != '\n';)
        {
            c = find_next_space(c);
            c = find_next_non_space(c);

            u32 draw_num = str_to_num(c);
            printf("%u ", draw_num);

            c = find_next_space(c);
            c = find_next_non_space(c);

            if(str_cmp(c, "red"))
            {
                if(draw_num > red_max)   is_game_valid = 0;
                if(draw_num > red_drawn) red_drawn     = draw_num;

                printf("red");
            }
            if(str_cmp(c, "green"))
            {
                if(draw_num > green_max)   is_game_valid = 0;
                if(draw_num > green_drawn) green_drawn   = draw_num;

                printf("green");
            }
            if(str_cmp(c, "blue"))
            {
                if(draw_num > blue_max)   is_game_valid = 0;
                if(draw_num > blue_drawn) blue_drawn    = draw_num;

                printf("blue");
            }
            
            c = find_next_non_letter(c);
            printf("%c", *c);
            if(*c != '\n') printf(" ");
        }

        u32 power = red_drawn * green_drawn * blue_drawn;
        power_sum += power;
        if(is_game_valid) 
        {
            game_sum += game_id;
        }
    }

    printf("SUM: %u\n", game_sum);
    printf("POWER SUM: %u\n", power_sum);

    printf("\nDONE\n");
    return 0;
}
