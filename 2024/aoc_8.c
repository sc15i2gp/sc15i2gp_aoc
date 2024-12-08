#include "util.h"

char test[] =
"............\n"
"........0...\n"
".....0......\n"
".......0....\n"
"....0.......\n"
"......A.....\n"
"............\n"
"............\n"
"........A...\n"
".........A..\n"
"............\n"
"............\n";

void print_map(char *map, s32 w, s32 h)
{
    for(s32 y = 0, i = 0; y < h; y += 1)
    {
        for(s32 x = 0; x < w; x += 1, i += 1)
        {
            printf("%c", map[i]);
        }
        printf("\n");
    }
}

u8 is_in_bounds(s32 x, s32 y, s32 w, s32 h)
{
    return (0 <= x) && (x < w) && (0 <= y) && (y < h);
}

void add_map_node_new(char *node_map, char *anti_map, s32 map_w, s32 map_h, char node, s32 nx, s32 ny, s32 p)
{
    for(s32 y = 0, i = 0; y < map_h; y += 1)
    {
        for(s32 x = 0; x < map_w; x += 1, i += 1)
        {
            if(node_map[i] == node)
            {
                anti_map[i] = '#';
                anti_map[p] = '#';

                s32 x_diff = x - nx;
                s32 y_diff = y - ny;

                for(s32 a0x = nx - x_diff, a0y = ny - y_diff;
                    is_in_bounds(a0x, a0y, map_w, map_h);
                    a0x -= x_diff, a0y -= y_diff)
                {
                    anti_map[a0y*map_w + a0x] = '#';
                }
                for(s32 a1x = x + x_diff, a1y = y + y_diff;
                    is_in_bounds(a1x, a1y, map_w, map_h);
                    a1x += x_diff, a1y += y_diff)
                {
                    anti_map[a1y*map_w + a1x] = '#';
                }
            }
        }
    }
    node_map[p] = node;
}

void add_map_node_old(char *node_map, char *anti_map, s32 map_w, s32 map_h, char node, s32 nx, s32 ny, s32 p)
{
    for(s32 y = 0, i = 0; y < map_h; y += 1)
    {
        for(s32 x = 0; x < map_w; x += 1, i += 1)
        {
            if(node_map[i] == node)
            {
                //Find difference in x, y between new node and i node
                s32 x_diff = x - nx;
                s32 y_diff = y - ny;

                //Find anti node positions
                s32 a0x = nx - x_diff;
                s32 a0y = ny - y_diff;
                s32 a1x = x + x_diff;
                s32 a1y = y + y_diff;

                //Check if they are in bounds, if so add them to anti_map
                if(is_in_bounds(a0x, a0y, map_w, map_h))
                {
                    anti_map[a0y*map_w + a0x] = '#';
                }
                if(is_in_bounds(a1x, a1y, map_w, map_h))
                {
                    anti_map[a1y*map_w + a1x] = '#';
                }
            }
        }
    }
    node_map[p] = node;
}

int main()
{
#if 1
    u32   in_file_size;
    char *in_file_contents = (char*)read_file_contents("aoc_8.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u32   in_file_size     = str_len(test);
#endif

    s32 map_w = 0;
    s32 map_h = 0;

    for(u32 i = 0; in_file_contents[i] != '\n'; i += 1, map_w += 1);
    for(u32 i = 0; i < in_file_size; i += 1) if(in_file_contents[i] == '\n') map_h += 1;

    u32   map_size = map_w * map_h;
    char *orig_map = (char*)alloc(map_size);

    for(s32 y = 0, i = 0; y < map_h; y += 1, i += 1)
    {
        for(s32 x = 0; x < map_w; x += 1, i += 1)
        {
            orig_map[i - y] = in_file_contents[i];
        }
    }
   
    char *node_map = (char*)alloc(map_size);
    char *anti_map = (char*)alloc(map_size);

    for(s32 i = 0; i < map_size; i += 1)
    {
        node_map[i] = '.';
        anti_map[i] = '.';
    }

    for(s32 y = 0, i = 0; y < map_h; y += 1)
    {
        for(s32 x = 0; x < map_w; x += 1, i += 1)
        {
            if(orig_map[i] != '.')
            add_map_node_old(node_map, anti_map, map_w, map_h, orig_map[i], x, y, i);
        }
    }

    u32 num_antis = 0;
    for(s32 i = 0; i < map_size; i += 1)
    {
        if(anti_map[i] == '#') num_antis += 1;
    }

    printf("Part 1: Num antis = %u\n", num_antis);

    for(s32 i = 0; i < map_size; i += 1)
    {
        node_map[i] = '.';
        anti_map[i] = '.'; 
    }
    
    for(s32 y = 0, i = 0; y < map_h; y += 1)
    {
        for(s32 x = 0; x < map_w; x += 1, i += 1)
        {
            if(orig_map[i] != '.')
            add_map_node_new(node_map, anti_map, map_w, map_h, orig_map[i], x, y, i);
        }
    }
    num_antis = 0;
    for(s32 i = 0; i < map_size; i += 1)
    {
        if(anti_map[i] == '#') num_antis += 1;
    }

    printf("Part 2: Num antis = %u\n", num_antis);
    return 0;
}
