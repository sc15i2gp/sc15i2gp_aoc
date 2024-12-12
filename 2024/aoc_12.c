#include "util.h"

//NOTE: This fuckin solution, man...

char test[] =
"RRRRIICCFF\n"
"RRRRIICCCF\n"
"VVRRRCCFFF\n"
"VVRCCCJFFF\n"
"VVVVCJJCFE\n"
"VVIVCCJJEE\n"
"VVIIICJJEE\n"
"MIIIIIJJEE\n"
"MIIISIJEEE\n"
"MMMISSJEEE\n";

void print_map(char *map, s64 w, s64 h)
{
    for(u64 y = 0, i = 0; y < h; y += 1)
    {
        for(u64 x = 0; x < w; x += 1, i += 1)
        {
            if(map[i]) printf("%c", map[i]);
            else       printf("%c", '0');
        }
        printf("\n");
    }
}

void replace_plants(char *map, s64 s, char lf, char rp)
{
    for(s64 i = 0; i < s; i += 1) if(map[i] == lf) map[i] = rp;
}

void restore_plants(char *map, char *orig_map, s64 s, char lf)
{
    for(s64 i = 0; i < s; i += 1) if(map[i] == lf) map[i] = orig_map[i];
}

enum
{
    PLANT_NONE = 0,
    PLANT_ENC_COUNT = '!',
    PLANT_ENC_CHECK = '#',
    PLANT_IN_REGION = '-',
    PLANT_TRAVERSED = '.',
    PLANT_START = 'A',
    PLANT_END   = 'Z',
    PLANT_COUNT
};

void find_region(char *map, s64 map_w, s64 sp, s64 *area, s64 *perimeter)
{
    char c = map[sp];
    *area += 1;
    map[sp] = PLANT_IN_REGION;
    
    //Above
    s64  np = sp - map_w;
    char nc = map[np];
    if(nc == c) find_region(map, map_w, np, area, perimeter);
    else if(nc != PLANT_IN_REGION) *perimeter += 1;
    
    //Below
    np = sp + map_w;
    nc = map[np];
    if(nc == c) find_region(map, map_w, np, area, perimeter);
    else if(nc != PLANT_IN_REGION) *perimeter += 1;

    //Left
    np = sp - 1;
    nc = map[np];
    if(nc == c) find_region(map, map_w, np, area, perimeter);
    else if(nc != PLANT_IN_REGION) *perimeter += 1;

    //Right
    np = sp + 1;
    nc = map[np];
    if(nc == c) find_region(map, map_w, np, area, perimeter);
    else if(nc != PLANT_IN_REGION) *perimeter += 1;
}

u8 region_enclosed(char *map, s64 map_w, s64 sp, char lf)
{
    map[sp] = PLANT_ENC_CHECK;

    //Above
    s64 np = sp - map_w;
    char n = map[np];
    if(n == lf)
    {
        if(!region_enclosed(map, map_w, np, lf)) return 0;
    }
    else if(n != PLANT_IN_REGION && n != PLANT_ENC_CHECK)
    {
        return 0;
    }

    //Below
    np = sp + map_w;
    n = map[np];
    if(n == lf)
    {
        if(!region_enclosed(map, map_w, np, lf)) return 0;
    }
    else if(n != PLANT_IN_REGION && n != PLANT_ENC_CHECK)
    {
        return 0;
    }

    //Left
    np = sp - 1;
    n = map[np];
    if(n == lf)
    {
        if(!region_enclosed(map, map_w, np, lf)) return 0;
    }
    else if(n != PLANT_IN_REGION && n != PLANT_ENC_CHECK)
    {
        return 0;
    }

    //Right
    np = sp + 1;
    n = map[np];
    if(n == lf)
    {
        if(!region_enclosed(map, map_w, np, lf)) return 0;
    }
    else if(n != PLANT_IN_REGION && n != PLANT_ENC_CHECK)
    {
        return 0;
    }

    return 1;
}

s64 rotate_cw(s64 dp, s64 w)
{
    //Right
    if(dp == 1)
    {
        dp = w;
    }
    //Down
    else if(dp == w)
    {
        dp = -1;
    }
    //Left
    else if(dp == -1)
    {
        dp = -w;
    }
    //Up
    else if(dp == -w)
    {
        dp = 1;
    }

    return dp;
}

s64 rotate_ccw(s64 dp, s64 w)
{
    //Right
    if(dp == 1)
    {
        dp = -w;
    }
    //Up
    else if(dp == -w)
    {
        dp = -1;
    }
    //Left
    else if(dp == -1)
    {
        dp = w;
    }
    //Down
    else if(dp == w)
    {
        dp = 1;
    }

    return dp;
}

//Traverse region edges with region to the right
s64 count_sides_ext(char *map, s64 map_w, s64 sp, char lf)
{
    sp -= map_w;
    s64 pp = sp;
    s64 dp = 1;
    s64 rp = map_w;

    s64 sides = 0;
    u8  moved = 0;

    do
    {
        //If map to the right is a plant and map in direction isn't
        if(map[pp+rp] == lf && map[pp+dp] != lf)
        {
            pp += dp;
            moved = 1;
        }
        else
        {
            if(map[pp+rp] != lf)
            {
                dp = rotate_cw(dp, map_w);
                rp = rotate_cw(rp, map_w);
                pp += dp;
                sides += 1;
            }
            else
            {
                dp = rotate_ccw(dp, map_w);
                rp = rotate_ccw(rp, map_w);
                sides += 1;
            }
        }
    }
    while(pp != sp || !moved);

    return sides;
}

s64 count_sides_int(char *map, s64 map_w, s64 map_h, s64 map_size)
{
    s64 sides = 0;
    for(s64 y = 1, i = map_w+1; y < map_h-1; y += 1, i += 2)
    {
        for(s64 x = 1; x < map_w-1; x += 1, i += 1)
        {
            char c = map[i];
            if(c != PLANT_NONE && c != PLANT_IN_REGION && c != PLANT_TRAVERSED && c != PLANT_ENC_CHECK && c != PLANT_ENC_COUNT)
            {
                if(map[i-1]     == PLANT_IN_REGION || map[i+1]     == PLANT_IN_REGION ||
                   map[i-map_w] == PLANT_IN_REGION || map[i+map_w] == PLANT_IN_REGION)
                {
                    u8 is_enclosed = region_enclosed(map, map_w, i, c);
                    if(is_enclosed)
                    {
                        sides += count_sides_ext(map, map_w, i, PLANT_ENC_CHECK);
                    }
                    replace_plants(map, map_size, PLANT_ENC_CHECK, PLANT_ENC_COUNT);
                }
            }
        }
    }
    return sides;
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = read_file_contents("aoc_12.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
#endif

    s64 map_w = 0;
    s64 map_h = 0;
    for(; in_file_contents[map_w] != '\n'; map_w += 1);
    for(u64 i = 0; i < in_file_size; i += 1) if(in_file_contents[i] == '\n') map_h += 1;

    map_w += 2;
    map_h += 2;

    u64 map_size   = (u64)(map_w * map_h);
    char *map      = (char*)alloc(map_size);
    char *orig_map = (char*)alloc(map_size);
    for(u64 y = 1; y < map_h - 1; y += 1)
    {
        char *in_row  = &in_file_contents[(y-1)*(map_w-1)];
        char *map_row = &map[y*map_w];
        for(u64 x = 1; x < map_w - 1; x += 1)
        {
            map_row[x] = in_row[x-1];
        }
    }

    for(u64 i = 0; i < map_size; i += 1) orig_map[i] = map[i];

    s64 price_sum_0 = 0;
    s64 price_sum_1 = 0;
    for(s64 y = 1, i = map_w+1; y < map_h-1; y += 1, i += 2)
    {
        for(s64 x = 1; x < map_w-1; x += 1, i += 1)
        {
            if(map[i] != PLANT_TRAVERSED)
            {
                s64  area  = 0;
                s64  perim = 0;
                s64  sides = 0;
                char c     = map[i];

                find_region(map, map_w, i, &area, &perim);
                sides += count_sides_ext(map, map_w, i, PLANT_IN_REGION);
                sides += count_sides_int(map, map_w, map_h, map_size);

                s64 price_0 = area * perim;
                price_sum_0 += price_0;
                
                s64 price_1 = area * sides;
                price_sum_1 += price_1;

                replace_plants(map, map_size, PLANT_IN_REGION, PLANT_TRAVERSED);
                restore_plants(map, orig_map, map_size, PLANT_ENC_COUNT);
            }
        }
    }
    printf("Part 1: Price sum = %ld\n", price_sum_0);
    printf("Part 2: Price sum = %ld\n", price_sum_1);
    return 0;
}
