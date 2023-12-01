#include <stdio.h>
#include <stdint.h>

typedef uint32_t u32;

const char in_arg[] =
#if 0
"1abc2\n"
"pqr3stu8vwx\n"
"a1b2c3d4e5f\n"
"treb7uchet\n";
#else
#include "aoc_2023_1.txt"
;
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

u32 is_number(const char *c)
{
    u32 is_digit_char = ('0' <= *c && *c <= '9');
    if(is_digit_char) return is_digit_char;

    if(str_cmp(c, "zero")  ||
       str_cmp(c, "one")   ||
       str_cmp(c, "two")   ||
       str_cmp(c, "three") ||
       str_cmp(c, "four")  ||
       str_cmp(c, "five")  ||
       str_cmp(c, "six")   ||
       str_cmp(c, "seven") ||
       str_cmp(c, "eight") ||
       str_cmp(c, "nine"))
    {
       return 1;
    }

    return 0;
}

u32 str_to_digit(const char *str)
{
    u32 is_digit_char = ('0' <= *str && *str <= '9');
    if(is_digit_char) return *str - '0';

    if(str_cmp(str, "zero"))  return 0;
    if(str_cmp(str, "one"))   return 1;
    if(str_cmp(str, "two"))   return 2;
    if(str_cmp(str, "three")) return 3;
    if(str_cmp(str, "four"))  return 4;
    if(str_cmp(str, "five"))  return 5;
    if(str_cmp(str, "six"))   return 6;
    if(str_cmp(str, "seven")) return 7;
    if(str_cmp(str, "eight")) return 8;
    if(str_cmp(str, "nine"))  return 9;
}

int main()
{
    u32 calibration_sum = 0;
    u32 in_arg_length = sizeof(in_arg) - 1; //-1 to ignore '\0'
    const char *in_end = in_arg + in_arg_length;
    for(const char *c = in_arg; c < in_end; ++c)
    {
        for(c; !is_number(c); ++c);

        const char *first_num = c;
        const char *last_num = c;

        for(c; *c != '\n'; ++c)
        {
            if(is_number(c)) last_num = c;
        }

        u32 first_digit = str_to_digit(first_num);
        u32 last_digit  = str_to_digit(last_num);
        printf("First %u Last %u\n", first_digit, last_digit);

        calibration_sum += 10 * (first_digit) + last_digit;
    }

    printf("Calibration sum = %u\n", calibration_sum);
    return 0;
}
