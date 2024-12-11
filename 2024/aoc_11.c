#include "util.h"

u64 test[]       = {125, 17};
u64 aoc_11_txt[] = {6571, 0, 5851763, 526746, 23, 69822, 9, 989};

void print_arr(u64 *arr, u64 arr_len)
{
    for(u64 i = 0; i < arr_len; i += 1) printf("%lu ", arr[i]);
}

u64 num_digits(u64 n)
{
    u64 d = 1;
    for(; n >= 10; n /= 10, d += 1);

    return d;
}

u8 has_even_digits(u64 n)
{
    u64 d = num_digits(n);
    return !(d & 0x1);
}

void split_digits(u64 n, u64 *n0, u64 *n1)
{
    u64 d = num_digits(n)/2;

    u64 r;
    for(r = 1; d > 0; r *= 10, d -= 1);

    *n0 = n / r;
    *n1 = n % r;
}

typedef struct
{
    u64 num;
    u64 count;
} arr_el;

void init_arr_el(arr_el *arr, u64 *input, u64 input_len)
{
    for(u64 i = 0; i < input_len; i += 1)
    {
        arr[i].num   = input[i];
        arr[i].count = 1;
    }
}

u64 count_arr_el(arr_el *arr, u64 arr_len)
{
    u64 count = 0;
    for(u64 i = 0; i < arr_len; i += 1) count += arr[i].count;

    return count;
}

//Returns new array length
u64 insert(arr_el *arr, u64 arr_len, u64 new_num, u64 new_count)
{
    for(u64 i = 0; i < arr_len; i += 1)
    {
        if(arr[i].num == new_num)
        {
            arr[i].count += new_count;
            return arr_len;
        }
    }

    arr[arr_len].num   = new_num;
    arr[arr_len].count = new_count;
    arr_len += 1;

    return arr_len;
}

u64 iter(arr_el *curr_arr, arr_el *next_arr, u64 curr_len)
{
    u64 n = 0;
    for(u64 i = 0; i < curr_len; i += 1)
    {
        u64 c = curr_arr[i].num;
        if(c == 0)
        {
            n = insert(next_arr, n, 1, curr_arr[i].count);
        }
        else if(has_even_digits(c))
        {
            u64 c0;
            u64 c1;
            split_digits(c, &c0, &c1);
            n = insert(next_arr, n, c0, curr_arr[i].count);
            n = insert(next_arr, n, c1, curr_arr[i].count);
        }
        else
        {
            n = insert(next_arr, n, 2024 * c, curr_arr[i].count);
        }
    }

    return n;
}

int main()
{
#if 1
    u64 *input     = aoc_11_txt;
    u64  input_len = 8;
#else
    u64 *input     = test;
    u64  input_len = 2;
#endif

    u64 arr_cap     = 128 * 1024 * 1024;
    arr_el *arr_0   = (arr_el*)alloc(arr_cap * sizeof(arr_el));
    arr_el *arr_1   = (arr_el*)alloc(arr_cap * sizeof(arr_el));

    u64  arr_len    = input_len;
    arr_el *arr     = arr_0;
    arr_el *next    = arr_1;

    init_arr_el(arr, input, input_len);

    u64 iter_count = 25;
    for(u64 i = 0; i < iter_count; i += 1)
    {
        arr_len = iter(arr, next, arr_len);
        arr_el *tmp = arr;
        arr = next;
        next = tmp;
    }
    printf("Part 1: Num stones = %lu\n", count_arr_el(arr, arr_len));

    init_arr_el(arr, input, input_len);
    arr_len    = input_len;
    iter_count = 75;
    for(u64 i = 0; i < iter_count; i += 1)
    {
        arr_len = iter(arr, next, arr_len);
        arr_el *tmp = arr;
        arr = next;
        next = tmp;
    }
    printf("Part 2: Num stones = %lu\n", count_arr_el(arr, arr_len));
    return 0;
}
