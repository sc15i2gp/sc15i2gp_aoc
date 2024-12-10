#include "util.h"

char test[] =
"89010123\n"
"78121874\n"
"87430965\n"
"96549874\n"
"45678903\n"
"32019012\n"
"01329801\n"
"10456732\n";

void print_map(char *map, s64 w, s64 h)
{
    for(s64 y = 0, i = 0; y < h; y += 1)
    {
        for(s64 x = 0; x < w; x += 1, i += 1)
        {
            printf("%c", map[i]);
        }
        printf("\n");
    }
}

void clear_map(char *map, s64 w, s64 h)
{
    for(s64 y = 0, i = 0; y < h; y += 1)
    {
        for(s64 x = 0; x < w; x += 1, i += 1)
        {
            map[i] = '.';
        }
    }
}

u8 is_in_bounds(s32 x, s32 y, s32 w, s32 h)
{
    return (0 <= x) && (x < w) && (0 <= y) && (y < h);
}

u64 score_trailhead(char *map, char *alt_map, s64 map_w, s64 map_h, s64 sx, s64 sy, s64 sp)
{
    u64 score = 0;
    char level = map[sp];
    alt_map[sp] = level;

    if(level == '9') score += 1;
    else
    {
        //Above
        s64 x = sx;
        s64 y = sy - 1;
        s64 p = sp - map_w;
        if(is_in_bounds(x, y, map_w, map_h) && map[p] == (level + 1))
        {
            score += score_trailhead(map, alt_map, map_w, map_h, x, y, p);
        }

        //Below
        x = sx;
        y = sy + 1;
        p = sp + map_w;
        if(is_in_bounds(x, y, map_w, map_h) && map[p] == (level + 1))
        {
            score += score_trailhead(map, alt_map, map_w, map_h, x, y, p);
        }

        //Left
        x = sx - 1;
        y = sy;
        p = sp - 1;
        if(is_in_bounds(x, y, map_w, map_h) && map[p] == (level + 1))
        {
            score += score_trailhead(map, alt_map, map_w, map_h, x, y, p);
        }

        //Right
        x = sx + 1;
        y = sy;
        p = sp + 1;
        if(is_in_bounds(x, y, map_w, map_h) && map[p] == (level + 1))
        {
            score += score_trailhead(map, alt_map, map_w, map_h, x, y, p);
        }
    }

    return score;
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = (char*)read_file_contents("aoc_10.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
#endif

    s64 map_w = 0;
    s64 map_h = 0;
    for(u64 i = 0; in_file_contents[i] != '\n'; i += 1, map_w += 1);
    for(u64 i = 0; i < in_file_size; i += 1) if(in_file_contents[i] == '\n') map_h += 1;

    s64 map_size = map_w * map_h;
    char *map     = (char*)alloc(map_size);
    char *alt_map = (char*)alloc(map_size);
    for(s64 y = 0, i = 0; y < map_h; y += 1, i += 1)
    {
        for(s64 x = 0; x < map_w; x += 1, i += 1)
        {
            map[i-y] = in_file_contents[i];
        }
    }

    clear_map(alt_map, map_w, map_h);

    u64 rating_sum = 0;
    u64 score_sum = 0;
    for(s64 y = 0, i = 0; y < map_h; y += 1)
    {
        for(s64 x = 0; x < map_w; x += 1, i += 1)
        {
            if(map[i] == '0')
            {
                rating_sum += score_trailhead(map, alt_map, map_w, map_h, x, y, i);
                for(u64 j = 0; j < map_size; j += 1) if(alt_map[j] == '9') score_sum += 1;
                clear_map(alt_map, map_w, map_h);
            }
        }
    }
    printf("Part 1: Sum scores = %lu\n", score_sum);
    printf("Part 2: Sum ratings = %lu\n", rating_sum);
    return 0;
}
