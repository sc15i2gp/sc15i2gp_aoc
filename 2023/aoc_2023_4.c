#include <stdio.h>
#include <stdint.h>
#include <windows.h>

typedef uint32_t u32;

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

u32 is_number(const char *str)
{
    return '0' <= *str && *str <= '9';
}

u32 str_to_num(const char *str)
{
    u32 n = 0;
    u32 base = 1;

    const char *num_end = str;
    for(; is_number(num_end); ++num_end);
    --num_end;

    for(u32 digit; num_end >= str; --num_end)
    {
        digit =  *num_end - '0';
        n     += base * digit;
        base  *= 10;
    }

    return n;
}

const char in_arg[] =
#if 0
"Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53 \n"
"Card 2: 13 32 20 16 61 | 61 30 68 82 17 32 24 19 \n"
"Card 3:  1 21 53 59 44 | 69 82 63 72 16 21 14  1 \n"
"Card 4: 41 92 73 84 69 | 59 84 76 51 58  5 54 83 \n"
"Card 5: 87 83 26 28 32 | 88 30 70 12 93 22 82 36 \n"
"Card 6: 31 18 13 56 72 | 74 77 10 23 35 67 36 11 \n";
#else
#include "aoc_2023_4.txt"
#endif

typedef struct
{
    u32 num_wins;
    u32 *wins;
    u32 num_nums;
    u32 *nums;
} scratchcard;

void print_scratchcard(scratchcard *s, u32 id)
{
    printf("Card %u: ", id);
    for(u32 i = 0; i < s->num_wins; ++i)
    {
        printf("%u ", s->wins[i]);
    }
    printf("| ");
    for(u32 i = 0; i < s->num_nums; ++i)
    {
        printf("%u ", s->nums[i]);
    }
}

void sort_list_nums(u32 *nums, u32 len)
{
    for(u32 swapped = 1; swapped == 1;)
    {
        swapped = 0;
        for(u32 i = 0; i < len - 1; ++i)
        {
            if(nums[i] > nums[i+1])
            {
                u32 tmp = nums[i];
                nums[i] = nums[i+1];
                nums[i+1] = tmp;
                swapped = 1;
            }
        }
        len -= 1;
    }
}

void sort_scratchcard(scratchcard *s)
{
    sort_list_nums(s->wins, s->num_wins);
    sort_list_nums(s->nums, s->num_nums);
}

u32 find_scratchcard_matching_num(scratchcard *s)
{
    u32 matching = 0;
    for(u32 i = 0; i < s->num_wins; ++i)
    {
        u32 win_num = s->wins[i];
        for(u32 j = 0; j < s->num_nums && s->nums[j] <= win_num; ++j)
        {
            u32 check_num = s->nums[j];
            if(check_num == win_num) matching += 1;
        }
    }

    return matching;
}

u32 find_scratchcard_score(scratchcard *s)
{
    u32 score = 0;
    u32 matching = find_scratchcard_matching_num(s);
    if(matching > 0)
    {
        score = 1 << (matching - 1);
    }
    return score;
}

//Assume str starts at "Card n:"
void str_to_scratchcard(const char *str, scratchcard *s, u32 num_wins, u32 num_nums)
{
    s->num_wins = num_wins;
    s->num_nums = num_nums;

    str = find_next_char(str, ':');
    str = find_next_space(str);
    str = find_next_non_space(str);

    for(u32 i = 0; i < num_wins; ++i)
    {
        s->wins[i] = str_to_num(str);
        str = find_next_space(str);
        str = find_next_non_space(str);
    }

    str = find_next_char(str, '|');
    str = find_next_space(str);
    str = find_next_non_space(str);

    for(u32 i = 0; i < num_nums; ++i)
    {
        s->nums[i] = str_to_num(str);
        str = find_next_space(str);
        str = find_next_non_space(str);
    }
}

int main()
{
    u32 in_length = sizeof(in_arg) - 1;

    const char *in_end = in_arg + in_length;

    //Count num cards
    //Count num winners
    //Count num numbers
    u32 num_wins = 0;
    u32 num_nums = 0;
    u32 num_scratchcards = 0;
    const char *c = in_arg;
    c = find_next_char(c, ':');
    c = find_next_space(c);
    c = find_next_non_space(c);

    for(; *c != '|';)
    {
        num_wins += 1;
        c = find_next_space(c);
        c = find_next_non_space(c);
    }

    c = find_next_space(c);
    c = find_next_non_space(c);

    for(; *c != '\n';)
    {
        num_nums += 1;
        c = find_next_space(c);
        c = find_next_non_space(c);
    }

    for(; c < in_end; ++c)
    {
        num_scratchcards += 1;
        c = find_next_char(c, '\n');
    }

    printf("%u %u %u\n", num_wins, num_nums, num_scratchcards);

    u32 scratchcard_length = num_wins + num_nums;
    u32 *numbers_buffer = VirtualAlloc(NULL, scratchcard_length * sizeof(u32), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    u32 *score_buffer = VirtualAlloc(NULL, num_scratchcards * sizeof(u32), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    for(u32 i = 0; i < num_scratchcards; ++i)
    {
        score_buffer[i] = 1;
    }

    scratchcard s = {};
    s.num_wins = num_wins;
    s.num_nums = num_nums;
    s.wins = numbers_buffer;
    s.nums = numbers_buffer + num_wins;

    u32 score_total = 0;
    c = in_arg;
    for(u32 i = 0; i < num_scratchcards; ++i)
    {
        str_to_scratchcard(c, &s, num_wins, num_nums);
        sort_scratchcard(&s);
        print_scratchcard(&s, i);
        
        u32 score = find_scratchcard_score(&s);
        u32 matching = find_scratchcard_matching_num(&s);

        score_total += score;
        printf("Score: %u\n", score);

        for(u32 j = 1; j <= matching; ++j)
        {
            score_buffer[i+j] += score_buffer[i];
        }

        c = find_next_char(c, '\n') + 1;
    }
    printf("Total Score: %u\n", score_total);
    for(u32 i = 0; i < num_scratchcards; ++i)
    {
        printf("Card %u: %u total\n", i, score_buffer[i]);
    }
    u32 total_scratchcards = 0;
    for(u32 i = 0; i < num_scratchcards; ++i)
    {
        total_scratchcards += score_buffer[i];
    }
    printf("Total scratchcards: %u\n", total_scratchcards);
    return 0;
}
