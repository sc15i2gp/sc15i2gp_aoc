#include "util.h"

const char test[] =
"....#.....\n"
".........#\n"
"..........\n"
"..#.......\n"
".......#..\n"
"..........\n"
".#..^.....\n"
"........#.\n"
"#.........\n"
"......#...\n";

#define EMPTY    0b00000000
#define G_LEFT   0b00000001
#define G_RIGHT  0b00000010
#define G_UP     0b00000100
#define G_DOWN   0b00001000
#define BLCK     0b00010000
#define ALT_BLCK 0b00100000
#define START    0b01000000
#define END      0b10000000
#define G_MASK   0b00001111

void map_to_str(u8 *map, s32 map_w, s32 map_h, char *str)
{    
    u32 i = 0;
    u32 j = 0;
    for(s32 y = 0; y < map_h; y += 1, j += 1)
    {
        for(s32 x = 0; x < map_w; x += 1, i += 1, j += 1)
        {
            char c;
            u8 m = map[i];
            switch(m)
            {
                case EMPTY:    c = '.'; break;
                case BLCK:     c = '#'; break;
                case ALT_BLCK: c = 'X'; break;
                case G_LEFT:   c = '<'; break;
                case G_RIGHT:  c = '>'; break;
                case G_UP:     c = '^'; break;
                case G_DOWN:   c = 'v'; break;
                case START:    c = 'S'; break;
                case END:      c = 'E'; break;
                default:
                {
                    if((m & G_LEFT) && (m & G_RIGHT)) c = '-';
                    if((m & G_UP)   && (m & G_DOWN))  c = '|';
                    if(((m & G_LEFT)  && (m & G_UP))   ||
                       ((m & G_LEFT)  && (m & G_DOWN)) ||
                       ((m & G_RIGHT) && (m & G_UP))   ||
                       ((m & G_RIGHT) && (m & G_DOWN))
                      ) c = '+';
                    break;
                };
            }
            str[j] = c;
        }
        str[j] = '\n';
    }
}

void print_map(u8 *map, s32 map_w, s32 map_h)
{
    char *str = (char*)alloc((map_w+1)*map_h + 1);
    map_to_str(map, map_w, map_h, str);
    printf("%s\n", str);
    dealloc(str);
}

void write_map(u8 *map, s32 map_w, s32 map_h, const char *path)
{
    u32 str_size = (map_w+1)*map_h + 1;
    char *str = (char*)alloc(str_size);
    map_to_str(map, map_w, map_h, str);
    write_file_contents(path, str, str_size);
    dealloc(str);
}

//0 = guard exited
//1 = guard loop
u8 run_guard(u8 *map, s32 map_w, s32 map_h, s32 pos_x, s32 pos_y, s32 pos, s32 dir_x, s32 dir_y, u8 dir)
{
    u8 guard_loop = 0;
    while(pos_x > 0 && pos_x < map_w && pos_y > 0 && pos_y < map_h)
    {
        s32 next_x   = pos_x + dir_x;
        s32 next_y   = pos_y + dir_y;
        s32 next_pos = pos + dir_x + (dir_y * map_w);
        u8  m        = map[next_pos];
        if(m == BLCK || m == ALT_BLCK)
        { 
            if(dir == G_UP)
            {
                dir_y = 0;
                dir_x = 1;
                dir   = G_RIGHT;
            }
            else if(dir == G_DOWN)
            {
                dir_y = 0;
                dir_x = -1;
                dir   = G_LEFT;
            }
            else if(dir == G_LEFT)
            {
                dir_x = 0;
                dir_y = -1;
                dir   = G_UP;
            }
            else if(dir == G_RIGHT)
            {
                dir_x = 0;
                dir_y = 1;
                dir   = G_DOWN;
            }
        }
        else
        {
            u8 p = map[pos];
            if(p == START) map[pos] = dir;
            else           map[pos] |= dir;
            pos_x = next_x;
            pos_y = next_y;
            pos   = next_pos;

            if((m & dir) || (p & dir))
            {
                guard_loop = 1;
                break;
            }
        }
    }

    return guard_loop;
}

int main()
{
#if 1
    u32   in_file_size;
    char *in_file_contents = read_file_contents("aoc_6.txt", &in_file_size);
#else
    u32 in_file_size = str_len(test);
    char *in_file_contents = test;
#endif

    s32 map_w;
    s32 map_h;
    for(map_w = 0; in_file_contents[map_w] != '\n'; map_w += 1);
    for(u32 i = 0; i < in_file_size; i += 1)
    {
        if(in_file_contents[i] == '\n') map_h += 1;
    }
    map_h -= 1;

    s32   map_size = map_w * map_h;
    u8   *map      = (u8*)alloc(map_size);
    u8   *orig_map = (u8*)alloc(map_size);
    u8   *alt_map  = (u8*)alloc(map_size);

    for(u32 i = 0, j = 0; i < in_file_size; i += 1)
    {
        if(in_file_contents[i] != '\n')
        {
            u8 t;
            switch(in_file_contents[i])
            {
                case '.': t = EMPTY; break;
                case '#': t = BLCK;  break;
                case '^': t = START; break;
            }
            map[j]      = t;
            orig_map[j] = t;
            alt_map[j]  = t;
            j += 1;
        }
    }
   
    s32 start_x;
    s32 start_y;
    s32 start_pos = 0;
    for(start_y = 0; start_y < map_h; start_y += 1)
    {
        for(start_x = 0; start_x < map_w; start_x += 1, start_pos += 1)
        {
            if(map[start_pos] == START) break;
        }
        if(map[start_pos] == START) break;
    }

    s32 dir_x = 0;
    s32 dir_y = -1;
    u8  dir   = G_UP;
    s32 pos_x = start_x;
    s32 pos_y = start_y;
    s32 pos   = start_pos;

    run_guard(map, map_w, map_h, pos_x, pos_y, pos, dir_x, dir_y, dir);

    u32 num_visited = 0;
    for(s32 i = 0; i < map_size; i += 1)
    {
        if(map[i] & G_MASK) num_visited += 1;
    }
    printf("Part 1: Num visited = %u\n", num_visited);
    u32 num_alts = 0;
    for(u32 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u32 x = 0; x < map_w; x += 1, i += 1)
        {
            if((i != start_pos) && (map[i] & G_MASK))
            {
                alt_map[i] = ALT_BLCK;
                u8 guard_loop = run_guard(alt_map, map_w, map_h, pos_x, pos_y, pos, dir_x, dir_y, dir);
                if(guard_loop) num_alts += 1;
                for(u32 j = 0; j < map_size; j += 1)
                {
                    alt_map[j] = orig_map[j];
                }
            }
        }
    }
    printf("Part 2: Num alts = %u\n", num_alts);
    return 0;
}
