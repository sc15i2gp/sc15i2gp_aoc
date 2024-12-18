#include "util.h"

//NOTE: I seriously suck at programming. This was much harder than it probably should have been.

#if 1
char test[] =
"###############\n"
"#.......#....E#\n"
"#.#.###.#.###.#\n"
"#.....#.#...#.#\n"
"#.###.#####.#.#\n"
"#.#.#.......#.#\n"
"#.#.#####.###.#\n"
"#...........#.#\n"
"###.#.#####.#.#\n"
"#...#.....#.#.#\n"
"#.#.#.###.#.#.#\n"
"#.....#...#.#.#\n"
"#.###.#.#.#.#.#\n"
"#S..#.....#...#\n"
"###############\n";
#else
char test[] =
"#################\n"
"#...#...#...#..E#\n"
"#.#.#.#.#.#.#.#.#\n"
"#.#.#.#...#...#.#\n"
"#.#.#.#.###.#.#.#\n"
"#...#.#.#.....#.#\n"
"#.#.#.#.#.#####.#\n"
"#.#...#.#.#.....#\n"
"#.#.#####.#.###.#\n"
"#.#.#.......#...#\n"
"#.#.###.#####.###\n"
"#.#.#...#.....#.#\n"
"#.#.#.#####.###.#\n"
"#.#.#.........#.#\n"
"#.#.#.#########.#\n"
"#S#.............#\n"
"#################\n";
#endif

typedef struct
{
    u32 page_size;
    u32 reserved;
    u32 allocated;
    u32 committed;
    u8 *buffer;
} mem_arena;

void commit_mem(mem_arena *arena, u32 commit_size)
{
    u8 *base          = arena->buffer + arena->committed;
    commit_size       = (commit_size + (arena->page_size - 1)) & ~(arena->page_size - 1);
    arena->committed += commit_size;
    VirtualAlloc(base, commit_size, MEM_COMMIT, PAGE_READWRITE);
}

void init_mem_arena(mem_arena *arena, u32 reserve_size)
{
    u32 page_size    = 4096; //Windows specific - No large pages
    arena->buffer    = (u8*)VirtualAlloc(NULL, reserve_size, MEM_RESERVE, PAGE_READWRITE);
    arena->page_size = page_size;
    arena->reserved  = reserve_size;
    arena->committed = 0;
    arena->allocated = 0;
}

void delete_mem_arena(mem_arena *arena)
{
    VirtualFree(arena->buffer, 0, MEM_RELEASE);
    arena->reserved  = 0;
    arena->committed = 0;
    arena->allocated = 0;
}

u8 *arena_alloc(mem_arena *arena, u32 alloc_size)
{
    u32 new_allocated = arena->allocated + alloc_size;
    if(new_allocated > arena->committed)
    {
        u32 to_commit = new_allocated - arena->committed;
        commit_mem(arena, to_commit);
    }
    u8 *p = arena->buffer + arena->allocated;
    arena->allocated += alloc_size;
    return p;
}

void arena_dealloc(mem_arena *arena, u32 dealloc_size)
{
    arena->allocated -= dealloc_size;
}

enum
{
    SEARCHED = 'Q',
    EMPTY  = '.',
    WALL  = '#',
    START = 'S',
    END   = 'E',
    U     = '^',
    D     = 'v',
    L     = '<',
    R     = '>',
    J     = 'J',
};

typedef struct
{
    u8  type;
    u8  dir;
    u8  on_best_path;
    u8  check_adj;
    u8  num_adj;
    u32 num_tiles;
    u32 num_turns;
    u32 cost;
    u32 x;
    u32 y;
} map_node;

void print_map_node(map_node *n)
{
    printf("NODE: %c %u\n", n->type, n->cost);
}

void print_map(map_node *map, u32 map_w, u32 map_h)
{
    char map_line[256] = {0};
    for(u32 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u32 x = 0; x < map_w; x += 1, i += 1)
        {
            char c = map[i].type;
            if(c == EMPTY && map[i].cost != 0) c = map[i].dir;
            map_line[x] = c;
        }
        printf("%.*s\n", map_w, map_line);
    }
}

void print_map_best(map_node *map, u32 map_w, u32 map_h)
{
    char map_line[256] = {0};
    for(u32 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u32 x = 0; x < map_w; x += 1, i += 1)
        {
            char c = map[i].type;
            if(c == EMPTY && map[i].on_best_path != 0) c = map[i].dir;
            map_line[x] = c;
        }
        printf("%.*s\n", map_w, map_line);
    }
}

map_node *get_node(map_node *map, u32 w, u32 x, u32 y)
{
    return &map[w*y + x];
}

typedef struct
{
    map_node *n;
    u32       p;
    u8        d;
} queue_entry;

typedef struct
{
    mem_arena    backing_mem;
    u32          capacity;
    u32          num_entries;
    queue_entry *entries;
    queue_entry *end;
} search_queue;

u8 is_empty(search_queue *q)
{
    return q->num_entries == 0;
}

void push_to_queue(search_queue *q, map_node *n, u8 d, u32 p)
{
    if(q->num_entries == q->capacity)
    {//Alloc space for 256 new entries
        u32 new_space    = 256 * sizeof(queue_entry);
        if(q->entries == NULL)
        {
            init_mem_arena(&q->backing_mem, 256 * 1024 * 1024);
            q->entries   = (queue_entry*)q->backing_mem.buffer;
            q->end       = q->entries;
        }
        q->capacity += 256;
        arena_alloc(&q->backing_mem, new_space);
    }
    queue_entry *e;
    if(q->num_entries != 0)
    {
        for(e = q->end; (e-1)->p > p; e -= 1);
        
        u32 mov_size = (q->end - e) * sizeof(queue_entry);
        memmove(e+1, e, mov_size);
    }
    else e = q->entries;

    e->n = n;
    e->d = d;
    e->p = p;
    q->num_entries += 1;
    q->end += 1;
}

map_node *pop_from_queue(search_queue *q, u8 *d)
{
    queue_entry *e = &q->entries[0];
    map_node    *n = e->n;
    if(d)       *d = e->d;

    q->num_entries -= 1;
    q->end         -= 1;
    u32 mov_size    = q->num_entries * sizeof(queue_entry);
    memmove(e, e+1, mov_size);

    return n;
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = read_file_contents("aoc_16.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
#endif

    u32 map_w = 0, map_h = 0;
    for(u32 i = 0; in_file_contents[i] != '\n'; i += 1, map_w += 1);
    for(u32 i = 0; i < in_file_size; i += 1) if(in_file_contents[i] == '\n') map_h += 1;
    printf("w = %u h = %u\n", map_w, map_h);

    u32 map_size = map_w * map_h;
    map_node *map = (map_node*)alloc(map_size * sizeof(map_node));
    for(u32 y = 0, i = 0; y < map_h; y += 1, i += 1)
    {
        for(u32 x = 0; x < map_w; x += 1, i += 1)
        {
            map_node *dst = &map[i-y];
            dst->type     = in_file_contents[i];
            dst->cost     = 0;
            dst->x        = x;
            dst->y        = y;
        }
    }

    //print_map(map, map_w, map_h); printf("\n");

    u32 sx, sy, ex, ey;
    for(u32 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u32 x = 0; x < map_w; x += 1, i += 1)
        {
            if(map[i].type == START) {sx = x; sy = y;}
            if(map[i].type == END)   {ex = x; ey = y;}
        }
    }
    printf("sx = %u sy = %u\n", sx, sy);
    printf("ex = %u ey = %u\n", ex, ey);
    map_node *s = get_node(map, map_w, sx, sy);
    s->cost = 0;
    s->dir  = R;

    search_queue q = {0};
    push_to_queue(&q, get_node(map, map_w, sx, sy), R, 0);
    do
    {
        map_node *n = pop_from_queue(&q, NULL);
        map_node *ma[4];
        u8        da[4];
        ma[0] = get_node(map, map_w, n->x-1, n->y); //Left
        ma[1] = get_node(map, map_w, n->x+1, n->y); //Right
        ma[2] = get_node(map, map_w, n->x, n->y-1); //Above
        ma[3] = get_node(map, map_w, n->x, n->y+1); //Below
        //da[0] = R; da[1] = L; da[2] = D; da[3] = U;
        da[0] = L; da[1] = R; da[2] = U; da[3] = D;

        for(u8 i = 0; i < 4; i += 1)
        {
            map_node *m = ma[i];
            u8        d = da[i];
            if(m->type != WALL && !n->check_adj) n->num_adj += 1;
            if(m->type != WALL && m->type != START)
            {
                u32 new_cost = (n->dir == d) ? n->cost + 1 : n->cost +  1001;
                if(m->cost == 0 || m->cost > new_cost)
                {
                    m->cost = new_cost;
                    m->dir  = d;
                    m->num_tiles = n->num_tiles + 1;
                    m->num_turns = (n->dir == d) ? n->num_turns : n->num_turns + 1;
                    push_to_queue(&q, m, 0, new_cost);
                }
            }
        }
        n->check_adj = 1;
    }
    while(!is_empty(&q));
    map_node *end = get_node(map, map_w, ex, ey);

    printf("%u %u\n", end->num_tiles, end->num_turns);
    u32 cost = end->num_tiles + 1000*end->num_turns;
    printf("Part 1: Score = %u\n", cost);
    print_map(map, map_w, map_h);
    map_node *f = get_node(map, map_w, ex-1, ey);
    printf("%u %u\n", f->num_tiles, f->num_turns);

    search_queue r = {0};
    push_to_queue(&r, get_node(map, map_w, ex, ey), 0, 0);
    do
    {
        u8        id;
        map_node *n = pop_from_queue(&r, &id);
        map_node *ma[4];
        u8        da[4];
        ma[0] = get_node(map, map_w, n->x-1, n->y); //Left
        ma[1] = get_node(map, map_w, n->x+1, n->y); //Right
        ma[2] = get_node(map, map_w, n->x, n->y-1); //Above
        ma[3] = get_node(map, map_w, n->x, n->y+1); //Below
        da[0] = L; da[1] = R; da[2] = U; da[3] = D;
        
        if(n->type == END)
        {
            n->on_best_path = 1;
            for(u8 i = 0; i < 4; i += 1)
            {
                map_node *m = ma[i];
                u8        d = da[i];
                if(m->num_tiles == n->num_tiles-1 && m->num_turns == n->num_turns)
                {
                    m->on_best_path = 1;
                    push_to_queue(&r, m, d, 0);
                }
            }
        }
        else
        {
            for(u8 i = 0; i < 4; i += 1)
            {
                map_node *m = ma[i];
                u8        d = da[i];
                if(d == id && m->num_tiles == n->num_tiles-1)
                {
                    m->on_best_path = 1;
                    push_to_queue(&r, m, d, 0);
                }
                else if(m->num_tiles == n->num_tiles-1 && m->num_turns == n->num_turns)
                {
                    m->on_best_path = 1;
                    push_to_queue(&r, m, d, 0);
                }
            }
        }
    }
    while(!is_empty(&r));

    printf("\n");
    print_map_best(map, map_w, map_h);

    u32 num_best = 0;
    for(u32 i = 0; i < map_size; i += 1) if(map[i].on_best_path) num_best += 1;
    printf("Part 2: Num best = %u\n", num_best);

    u32 n[5] = {0};
    for(u32 i = 0; i < map_size; i += 1) n[map[i].num_adj] += 1;
    printf("N1: %u N2: %u N3: %u N4: %u\n", n[1], n[2], n[3], n[4]);
    return 0;
}
