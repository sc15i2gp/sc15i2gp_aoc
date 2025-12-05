#include "util.h"

u64 log_2(u64 n)
{
    if(n) return 32 - __builtin_clz(n);
    else  return 0;
}

u64 base_10[] =
{0,
 0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,
 5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9,
 9,  10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14,
 14, 15, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 18, 19};
u64 powers_10[] =
{1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL, 100000000ULL,
 1000000000ULL, 10000000000ULL, 100000000000ULL, 1000000000000ULL,
 10000000000000ULL, 100000000000000ULL, 1000000000000000ULL,
 10000000000000000ULL, 100000000000000000ULL, 1000000000000000000ULL,
// 10000000000000000000ULL, 100000000000000000000ULL, 1000000000000000000000ULL
 };
u64 log_10(u64 n)
{
    u64 l = base_10[log_2(n)];
    if(n >= powers_10[l]) l += 1;
    return l;
}

u8 is_seq_rep(u64 n, u64 m, u64 b)
{
    // If n has an odd number of digits it cannot be a sequence repeated twice
    if(b % m != 0) return 0;
    b /= m;
    u64 p = powers_10[b];
    u64 sub[20]; // Assume 20 max patterns in a number (i.e. any number is < 10^20)
    for(u32 i = 0; i < m; i += 1) { sub[i] = n % p; n /= p; }
    u64 s = sub[0];
    for(u32 i = 1; i < m; i += 1) if(sub[i] != s) return 0;
    return 1;
}

u8 is_seq_rep_2(u64 n)
{
    u64 b = log_10(n);
    // If n has an odd number of digits it cannot be a sequence repeated twice
    if(b & 0b1UL) return 0;
    b >>= 1;
    u64 p = powers_10[b];
    u64 q = n / p;
    u64 r = n % p;
    return q == r;
}

u64 next_invalid_id_or_max(u64 min, u64 max)
{
    u64 ret = max;
    for(u64 i = min; i < max; i += 1)
    {
        u64 b = log_10(i);
        for(u32 m = 2; m <= b; m += 1)
        {
            if(is_seq_rep(i, m, b)) { ret = i; goto done; }
        }
    }
done:
    return ret;
}

u64 next_invalid_id_or_max_2(u64 min, u64 max)
{
    u64 ret = max;
    for(u64 i = min; i < max; i += 1)
    {
        if(is_seq_rep_2(i)) { ret = i; break; }
    }
    return ret;
}

typedef struct
{
    u32 num_entries;
    u64 *minima;
    u64 *maxima;
} ranges;

void print_ranges(ranges *r)
{
    for(u32 i = 0; i < r->num_entries; i += 1)
    {
        printf("%llu|%llu,", r->minima[i], r->maxima[i]);
    }
    printf("\n");
}

u64 invalid_id_sum(ranges *r, u8 only_twice)
{
    u64 sum = 0;
    for(u32 i = 0; i < r->num_entries; i += 1)
    {
        u64 s = r->minima[i], l = r->maxima[i];
        for(u64 s = r->minima[i], l = r->maxima[i];
            s < l; s += 1)
        {
            if(only_twice)
            { s = next_invalid_id_or_max_2(s, l);}
            else
            { s = next_invalid_id_or_max(s, l);} 
            if(s < l) sum += s;
        }
    }
    return sum;
}

ranges read_input(const char *in, u32 in_len)
{
    tokeniser to = init_tokeniser(in, in_len);
    u32 num_entries = 0;
    for(token t = read_token(&to); t.type != TOKEN_END; t = read_token(&to))
    {
        if(t.type == '|') num_entries += 1;
    }
    u64 *minima = alloc(num_entries * sizeof(u64));
    u64 *maxima = alloc(num_entries * sizeof(u64));
    to = init_tokeniser(in, in_len);
    for(u32 i = 0; i < num_entries; i += 1)
    {
        token t;
        t = read_token(&to); read_token(&to);
        minima[i] = (u64)t.int_val;
        t = read_token(&to); read_token(&to);
        maxima[i] = ((u64)t.int_val) + 1;
    }
    
    ranges r = {.num_entries = num_entries, .minima = minima, .maxima = maxima};
    return r;
}

const char example[] =
{
    "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,"
    "1698522-1698528,446443-446449,38593856-38593862,565653-565659,"
    "824824821-824824827,2121212118-2121212124"
};

int main()
{
#if 0
    const char *_in = example;
    u32 in_len = strlen(_in);
#else
    u64 in_len;
    char *_in = read_file_contents("aoc_2.txt", &in_len);
#endif
    char *in = alloc(in_len);
    for(u32 i = 0; i < in_len; i += 1)
    {
        if(_in[i] == '-') in[i] = '|';
        else              in[i] = _in[i];
    }
    ranges r = read_input(in, in_len);
    printf("INVALID ID SUM = %llu\n", invalid_id_sum(&r, 1));
    printf("INVALID ID SUM = %llu\n", invalid_id_sum(&r, 0));
    printf("DONE!\n");
    return 0;
}
