#include <stdio.h>
#include <stdint.h>
#include <windows.h>

typedef uint64_t u64;

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

u64 is_number(const char *str)
{
    return '0' <= *str && *str <= '9';
}

u64 str_to_num(const char *str)
{
    u64 n = 0;
    u64 base = 1;

    const char *num_end = str;
    for(; is_number(num_end); ++num_end);
    --num_end;

    for(u64 digit; num_end >= str; --num_end)
    {
        digit =  *num_end - '0';
        n     += base * digit;
        base  *= 10;
    }

    return n;
}

typedef struct
{
    u64 dst_start;
    u64 src_start;
    u64 length;
} map_range;

typedef struct
{
    u64 num_ranges;
    map_range *ranges;
} map;

u64 find_map_value(map *m, u64 src)
{
    for(u64 i = 0; i < m->num_ranges; ++i)
    {
        map_range *r = &m->ranges[i];
        u64 src_max = r->src_start + r->length;

        if(r->src_start <= src && src < src_max)
        {
            u64 diff = src - r->src_start;
            return r->dst_start + diff;
        }
    }
    return src;
}

void print_map(map *m, const char *src, const char *dst)
{
    printf("%s-to-%s map:\n");
    for(u64 i = 0; i < m->num_ranges; ++i)
    {
        map_range *r = &m->ranges[i];
        printf("%u %u %u\n", r->dst_start, r->src_start, r->length);
    }
}

const char in_arg[] =
#if 0
"seeds: 79 14 55 13 \n"
"\n"
"seed-to-soil map:\n"
"50 98 2\n"
"52 50 48\n"
"\n"
"soil-to-fertilizer map:\n"
"0 15 37\n"
"37 52 2\n"
"39 0 15\n"
"\n"
"fertilizer-to-water map:\n"
"49 53 8\n"
"0 11 42\n"
"42 0 7\n"
"57 7 4\n"
"\n"
"water-to-light map:\n"
"88 18 7\n"
"18 25 70\n"
"\n"
"light-to-temperature map:\n"
"45 77 23\n"
"81 45 19\n"
"68 64 13\n"
"\n"
"temperature-to-humidity map:\n"
"0 69 1\n"
"1 0 69\n"
"\n"
"humidity-to-location map:\n"
"60 56 37\n"
"56 93 4\n";
#else
#include "aoc_2023_5.txt"
#endif

int main()
{
    u64 in_length = sizeof(in_arg) - 1;
    const char *in_end = in_arg + in_length;

    const char *c = find_next_char(in_arg, ':');
    c = find_next_space(c);
    c = find_next_non_space(c);

    u64 seeds[32] = {};
    u64 num_seeds = 0;

    for(; *c != '\n';)
    {
        seeds[num_seeds] = str_to_num(c);
        ++num_seeds;

        c = find_next_space(c);
        c = find_next_non_space(c);
    }

    const char *maps_start = c+2;
    map almanac[7] = {};
    //map almanac[7] = {seed_soil_map, soil_fert_map, fert_water_map, water_light_map, light_temp_map, temp_humid_map, humid_loc_map};
    u64 num_maps = 7;

    u64 num_ranges = 0;
    //Count total num ranges
    for(u64 i = 0; i < num_maps; ++i)
    {
        c = find_next_char(c, ':');
        c = find_next_char(c, '\n');
        ++c;

        for(; c < in_end && *c != '\n'; ++c)
        {
            ++num_ranges;
            ++almanac[i].num_ranges;
            c = find_next_char(c, '\n');
        }
    }

    map_range *range_buffer = VirtualAlloc(NULL, num_ranges * sizeof(map_range), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    map_range *r = range_buffer;
    for(u64 i = 0; i < num_maps; ++i)
    {
        almanac[i].ranges = r;
        r += almanac[i].num_ranges;
    }

    c = maps_start;
    for(u64 i = 0; i < num_maps; ++i)
    {
        c = find_next_char(c, ':');
        c = find_next_char(c, '\n');
        ++c;
        for(u64 j = 0; j < almanac[i].num_ranges; ++j)
        {
            almanac[i].ranges[j].dst_start = str_to_num(c);
            c = find_next_space(c);
            c = find_next_non_space(c);

            almanac[i].ranges[j].src_start = str_to_num(c);
            c = find_next_space(c);
            c = find_next_non_space(c);

            almanac[i].ranges[j].length = str_to_num(c);
            c = find_next_char(c, '\n');
            ++c;
        }
    }

    printf("Num ranges: %u\n", num_ranges);

    u64 lowest_loc = -1;
    for(u64 s = 0; s < num_seeds; ++s)
    {
        u64 seed_start = seeds[s];
        u64 seed_range = seeds[s+1];
        ++s;
        for(u64 j = 0; j < seed_range; ++j)
        {
            u64 n = seed_start + j;
            //printf("Seed %u ", n);
            for(u64 i = 0; i < num_maps; ++i)
            {
                n = find_map_value(&almanac[i], n);
                //printf("-> %u ", n);
            }
            if(n < lowest_loc) lowest_loc = n;
            //printf("\n");
        }
        printf("Done seed start %u\n", seed_start);
    }
    printf("Lowest location: %u\n", lowest_loc);
    return 0;
}
