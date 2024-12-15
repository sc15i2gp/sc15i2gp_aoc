#include "util.h"

#if 0
char test[] =
"########\n"
"#..O.O.#\n"
"##@.O..#\n"
"#...O..#\n"
"#.#.O..#\n"
"#...O..#\n"
"#......#\n"
"########\n"
"\n"
"<^^>>>vv<v>>v<<\n";
#else
char test[] =
"##########\n"
"#..O..O.O#\n"
"#......O.#\n"
"#.OO..O.O#\n"
"#..O@..O.#\n"
"#O#..O...#\n"
"#O..O..O.#\n"
"#.OO.O.OO#\n"
"#....O...#\n"
"##########\n"
"\n"
"<vv>^<v^>v>^vv^v>v<>v^v<v<^vv<<<^><<><>>v<vvv<>^v^>^<<<><<v<<<v^vv^v>^\n"
"vvv<<^>^v^^><<>>><>^<<><^vv^^<>vvv<>><^^v>^>vv<>v<<<<v<^v>^<^^>>>^<v<v\n"
"><>vv>v^v^<>><>>>><^^>vv>v<^^^>>v^v^<^^>v^^>v^<^v>v<>>v^v^<v>v^^<^^vv<\n"
"<<v<^>>^^^^>>>v^<>vvv^><v<<<>^^^vv^<vvv>^>v<^^^^v<>^>vvvv><>>v^<<^^^^^\n"
"^><^><>>><>^^<<^^v>>><^<v>^<vv>>v>>>^v><>^v><<<<v>>v<v<v>vvv>^<><<>^><\n"
"^>><>^v<><^vvv<^^<><v<<<<<><^v<<<><<<^^<v<^^^><^>>^<v^><<<^>>^v<v^v<v^\n"
">^>>^v>vv>^<<^v<>><<><<v<<v><>v<^vv<<<>^^v^>^^>>><<^v>>v^v><^^>>^<>vv^\n"
"<><^^>^^^<><vvvvv^v<v<<>^v<v>v<<^><<><<><<<^^<<<^<<>><<><^^^>^^<>^>v<>\n"
"^^>vv<^v^v<vv>^<><v<^v>^^^>>>^^vvv^>vvv<>>>^<^>>>>>^<<^v>^vvv<>^<><<v>\n"
"v^^>>><<^^<>>^v^<v^vv<>v^<<>^<^v^v><^<<<><<^<v><v<>vv>>v><v^<vv<>v^<<^\n";
#endif
enum
{
    NONE  = '.',
    WALL  = '#',
    BOX   = 'O',
    BOX_L = '[',
    BOX_R = ']',
    ROBOT = '@',
    L     = '<',
    R     = '>',
    U     = '^',
    D     = 'v'
};

void print_map(char *map, u64 map_w, u64 map_h)
{
    char map_line[256] = {0};
    for(u64 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u64 x = 0; x < map_w; x += 1, i += 1)
        {
            char c = map[i];
            switch(c)
            {
                case WALL:  c = '#'; break;
                case BOX:   c = 'O'; break;
                case BOX_L: c = '['; break;
                case BOX_R: c = ']'; break;
                case ROBOT: c = '@'; break;
                default:    c = '.'; break;
            }
            map_line[x] = map[i];
        }
        printf("%.*s\n", map_w, map_line);
    }
}

void init_map(char *map, u64 map_w, u64 map_h, char *in)
{
    for(u64 y = 0, i = 0; y < map_h; y += 1, i += 1)
    {
        for(u64 x = 0; x < map_w; x += 1, i += 1)
        {
            map[i-y] = in[i];
        }
    }
}

void move_box_u_or_d(char *map, s64 lb, s64 rb, s64 v)
{
    s64  n_lb = lb + v;
    s64  n_rb = rb + v;
    char c_lb = map[n_lb];
    char c_rb = map[n_rb];

    if(c_lb == BOX_L && c_rb == BOX_R)
    {
        move_box_u_or_d(map, n_lb, n_rb, v);
    }
    if(c_lb == BOX_R)
    {
        move_box_u_or_d(map, n_lb-1, n_lb, v);
    }
    if(c_rb == BOX_L)
    {
        move_box_u_or_d(map, n_rb, n_rb+1, v);
    }

    map[n_lb] = map[lb];
    map[n_rb] = map[rb];
    map[lb]   = NONE;
    map[rb]   = NONE;
}

u8 box_can_move_u_or_d(char *map, s64 lb, s64 rb, s64 v)
{
    u8 can_move = 1;
    s64  n_lb = lb + v;
    s64  n_rb = rb + v;
    char c_lb = map[n_lb];
    char c_rb = map[n_rb];

    if(c_lb == WALL || c_rb == WALL) return 0;

    if(c_lb == BOX_L && c_rb == BOX_R)
    {
        return box_can_move_u_or_d(map, n_lb, n_rb, v);
    }
    if(c_lb == BOX_R)
    {
        can_move = can_move && box_can_move_u_or_d(map, n_lb-1, n_lb, v);
    }
    if(c_rb == BOX_L)
    {
        can_move = can_move && box_can_move_u_or_d(map, n_rb, n_rb+1, v);
    }

    return can_move;
}

s64 move_robot(char *map, s64 op, s64 v)
{
    s64 np    = op + v;
    if(map[np] == NONE)
    {
        map[op]   = NONE;
        map[np]   = ROBOT;
        return np;
    }
    else if(map[np] == BOX)
    {
        //Find end of line of boxes
        s64 b = np;
        for(; map[b] == BOX; b += v);
        //If the last one can be pushed a space then push
        //Otherwise treat like a wall
        if(map[b] != WALL)
        {
            for(; b != np; b -= v)
            {
                char c = map[b-v];
                map[b]   = c;
                map[b-v] = NONE;
            }
            char c   = map[b-v];
            map[b]   = c;
            map[b-v] = NONE;
            return np;
        }
        else return op;
    }
    else if(map[np] == BOX_L || map[np] == BOX_R)
    {
        if(v == 1 || v == -1)
        {//Robot move L or R
            //Find end of line of boxes
            s64 b = np;
            for(; map[b] == BOX_L || map[b] == BOX_R; b += v);
            //If the last one can be pushed a space then push
            //Otherwise treat like a wall
            if(map[b] != WALL)
            {
                for(; b != np; b -= v)
                {
                    char c = map[b-v];
                    map[b]   = c;
                    map[b-v] = NONE;
                }
                char c   = map[b-v];
                map[b]   = c;
                map[b-v] = NONE;
                return np;
            }
            else return op;
        }
        else
        {//Robot move U or D
            u8 box_can_move;
            s64 lb = (map[np] == BOX_L) ? np : np-1;
            s64 rb = (map[np] == BOX_L) ? np+1 : np;
            box_can_move = box_can_move_u_or_d(map, lb, rb, v);
            if(box_can_move)
            {
                move_box_u_or_d(map, lb, rb, v);
                map[np] = map[op];
                map[op] = NONE;
                return np;
            }
            else return op;
        }
    }
    else return op;
}

void run_instrs(char *instrs, u64 instrs_len, char *map, u64 map_w, u64 map_h)
{
    s64 w = (s64)map_w;
    s64 h = (s64)map_h;
    s64 u = -w;
    s64 d = w;
    s64 l = -1;
    s64 r = 1;

    s64 x, y, p;
    for(y = 0, p = 0; y < h; y += 1)
    {
        for(x = 0; x < w; x += 1, p += 1)
        {
            if(map[p] == ROBOT) break;
        }
        if(map[p] == ROBOT) break;
    }

    for(u64 i = 0; i < instrs_len; i += 1)
    {
        char instr = instrs[i];
        switch(instr)
        {
            case U:
            {
                p = move_robot(map, p, u);
                break;
            }
            case D:
            {
                p = move_robot(map, p, d);
                break;
            }
            case L:
            {
                p = move_robot(map, p, l);
                break;
            }
            case R:
            {
                p = move_robot(map, p, r);
                break;
            }
            default: continue;
        }
    }
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = read_file_contents("aoc_15.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
#endif

    u64 map_w = 0, map_h = 0;
    for(u64 i = 0; in_file_contents[i] != '\n'; i += 1, map_w += 1);
    for(u64 i = 1; !(in_file_contents[i-1] == '\n' && in_file_contents[i] == '\n'); i += 1)
    {
        if(in_file_contents[i] == '\n') map_h += 1;
    }
    printf("W = %lu H = %lu\n", map_w, map_h);

    u64 map_size = map_w * map_h;
    char *map = (char*)alloc(map_size);
    init_map(map, map_w, map_h, in_file_contents);

    u64 instrs_len = in_file_size - (map_size + map_h + 1);
    char *instrs  = (char*)alloc(instrs_len);
    char *instr_start = in_file_contents + map_size + map_h + 1;
    for(u64 i = 0; i < instrs_len; i += 1) instrs[i] = instr_start[i];

    print_map(map, map_w, map_h); printf("\n");
    run_instrs(instrs, instrs_len, map, map_w, map_h);
    print_map(map, map_w, map_h); printf("\n");

    s64 sum_gps = 0;
    for(s64 y = 0, i = 0; y < (s64)map_h; y += 1)
    {
        for(s64 x = 0; x < (s64)map_w; x += 1, i += 1)
        {
            if(map[i] == BOX)
            {
                s64 gps = 100 * y + x;
                sum_gps += gps;
            }
        }
    }
    printf("Part 1: Sum gps = %ld\n", sum_gps);

    init_map(map, map_w, map_h, in_file_contents);

    map_w *= 2;
    u64 wmap_size   = 2 * map_size;
    char *wmap = (char*)alloc(wmap_size);
    for(u64 i = 0; i < map_size; i += 1)
    {
        char c = map[i];
        switch(c)
        {
            case WALL:  wmap[2*i] = WALL;  wmap[(2*i)+1] = WALL;  break;
            case BOX:   wmap[2*i] = BOX_L; wmap[(2*i)+1] = BOX_R; break;
            case NONE:  wmap[2*i] = NONE;  wmap[(2*i)+1] = NONE;  break;
            case ROBOT: wmap[2*i] = ROBOT; wmap[(2*i)+1] = NONE;  break;
        }
    }

    print_map(wmap, map_w, map_h); printf("\n");
    run_instrs(instrs, instrs_len, wmap, map_w, map_h);
    print_map(wmap, map_w, map_h); printf("\n");

    sum_gps = 0;
    for(s64 y = 0, i = 0; y < (s64)map_h; y += 1)
    {
        for(s64 x = 0; x < (s64)map_w; x += 1, i += 1)
        {
            if(wmap[i] == BOX_L)
            {
                s64 gps = 100 * y + x;
                sum_gps += gps;
            }
        }
    }
    printf("Part 2: Sum gps = %ld\n", sum_gps);
    return 0;
}
