#include "util.h"

#if 1
char test[] =
"p=0,4 v=3,-3\n"
"p=6,3 v=-1,-3\n"
"p=10,3 v=-1,2\n"
"p=2,0 v=2,-1\n"
"p=0,0 v=1,3\n"
"p=3,0 v=-2,-2\n"
"p=7,6 v=-1,-3\n"
"p=3,0 v=-1,-2\n"
"p=9,3 v=2,3\n"
"p=7,3 v=-1,2\n"
"p=2,4 v=2,-3\n"
"p=9,5 v=-3,-3\n";
#else
char test[] =
"p=10,3 v=1,0\n";

#endif
typedef struct
{
    s64 px;
    s64 py;
    s64 vx;
    s64 vy;
} robot;

void print_robot(robot *r)
{
    printf("p=%ld,%ld v=%ld,%ld", r->px, r->py, r->vx, r->vy);
}

robot *parse_robots(char *in, u64 in_size, u64 *ret_num_robots)
{
    tokeniser t = init_tokeniser(in, in_size);

    u64 num_robots = 0;
    for(token tok = read_token(&t); tok.type != TOKEN_END; tok = read_token(&t))
    {
        if(tok.type == TOKEN_NEWLINE) num_robots += 1;
    }

    robot *robots = (robot*)alloc(num_robots * sizeof(robot));
    t = init_tokeniser(in, in_size);

    for(u64 i = 0; i < num_robots; i += 1)
    {
        robot *dst = &robots[i];
        token tok;
        for(u64 j = 0; j < 3; j += 1) tok = read_token(&t);
        dst->px = tok.int_val;
        for(u64 j = 0; j < 2; j += 1) tok = read_token(&t);
        dst->py = tok.int_val;
        for(u64 j = 0; j < 4; j += 1) tok = read_token(&t);
        dst->vx = tok.int_val;
        for(u64 j = 0; j < 2; j += 1) tok = read_token(&t);
        dst->vy = tok.int_val;
        tok = read_token(&t);
    }

    *ret_num_robots = num_robots;
    return robots;
}

void print_map(char *map, u64 map_w, u64 map_h)
{
    char map_line[256] = {0};

    for(u64 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u64 x = 0; x < map_w; x += 1, i += 1)
        {
            char c = map[i];
            if(c == 0) c = '.';
            else       c += '0';
            map_line[x] = c;
        }
        printf("%.*s\n", map_w, map_line);
    }
}

void robots_to_map(robot *robots, u64 num_robots, char *map, u64 map_w, u64 map_h)
{    
    for(u64 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u64 x = 0; x < map_w; x += 1, i += 1) map[i] = 0;
    }

    for(u64 i = 0; i < num_robots; i += 1)
    {
        s64 px = robots[i].px;
        s64 py = robots[i].py;
        map[map_w*py + px] += 1;
    }
}

void robot_iter(robot *robots, u64 num_robots, u64 map_w, u64 map_h)
{
    s64 w = (s64)map_w;
    s64 h = (s64)map_h;
    for(u64 i = 0; i < num_robots; i += 1)
    {
        robot *r = &robots[i];
        r->px += r->vx;
        r->py += r->vy;

        if     (r->px < 0) r->px += w;
        else if(r->px >= w) r->px -= w;
        if     (r->py < 0) r->py += h;
        else if(r->py >= h) r->py -= h;
    }
}

u64 safety_factor(char *map, u64 map_w, u64 map_h)
{
    s64 hx = map_w/2; s64 hy = map_h/2;
    u64 quads[4] = {0};
    //Top left
    for(s64 y = 0; y < hy; y += 1)
    {
        for(s64 x = 0; x < hx; x += 1)
        {
            quads[0] += map[map_w * y + x];
        }
    }
    //Top right
    for(s64 y = 0; y < hy; y += 1)
    {
        for(s64 x = hx + 1; x < map_w; x += 1)
        {
            quads[1] += map[map_w * y + x];
        }
    }
    //Bottom left
    for(s64 y = hy + 1; y < map_h; y += 1)
    {
        for(s64 x = 0; x < hx; x += 1)
        {
            quads[2] += map[map_w * y + x];
        }
    }
    //Bottom right
    for(s64 y = hy + 1; y < map_h; y += 1)
    {
        for(s64 x = hx + 1; x < map_w; x += 1)
        {
            quads[3] += map[map_w * y + x];
        }
    }

    u64 safety_factor = quads[0] * quads[1] * quads[2] * quads[3];
    return safety_factor;
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = read_file_contents("aoc_14.txt", &in_file_size);
    u64   map_w = 101;
    u64   map_h = 103;
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
    u64   map_w = 11;
    u64   map_h = 7;
#endif

    u64 map_size = map_w * map_h;
    char *map = (char*)alloc(map_size);

    u64 num_robots;
    robot *robots = parse_robots(in_file_contents, in_file_size, &num_robots);

    u64 n = 100;
    for(u64 i = 0; i < n; i += 1)
    {
        robot_iter(robots, num_robots, map_w, map_h);
    }
    robots_to_map(robots, num_robots, map, map_w, map_h);
    printf("Part 1: Safety factor = %lu\n", safety_factor(map, map_w, map_h));

    robots = parse_robots(in_file_contents, in_file_size, &num_robots);
    u64 lowest_sf = safety_factor(map, map_w, map_h);
    n = 10403;
    for(u64 i = 0; i < n; i += 1)
    {
        robot_iter(robots, num_robots, map_w, map_h);
        robots_to_map(robots, num_robots, map, map_w, map_h);
        u64 sf = safety_factor(map, map_w, map_h);
        if(sf < lowest_sf)
        {
            printf("\nSeconds = %lu\n", i);
            print_map(map, map_w, map_h); printf("\n");
            lowest_sf = sf;
        }
    }

    //NOTE: Got this heuristically by doing the above checking for low safety
    //factor (since an ordered image like the X-mas tree would have smaller
    //distribution). The last image printed is the tree.
    printf("Part 2: Num seconds = %lu\n", 6888);
    return 0;
}
