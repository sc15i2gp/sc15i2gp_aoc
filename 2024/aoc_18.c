#include "util.h"

//NOTE: After day 16: was that really it?

char test[] =
"5,4\n"
"4,2\n"
"4,5\n"
"3,0\n"
"2,1\n"
"6,3\n"
"2,4\n"
"1,5\n"
"0,6\n"
"3,3\n"
"2,6\n"
"5,1\n"
"1,2\n"
"5,5\n"
"2,5\n"
"6,5\n"
"1,4\n"
"0,4\n"
"6,4\n"
"1,1\n"
"6,1\n"
"1,0\n"
"0,5\n"
"1,6\n"
"2,0\n";

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

typedef struct
{
    u8  type;
    u8  searched;
    u64 cost;
    s64 x;
    s64 y;
} map_node;

typedef struct
{
    map_node *n;
    u32       p;
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

void push_to_queue(search_queue *q, map_node *n, u32 p)
{
    if(q->num_entries == q->capacity)
    {//Alloc space for 256 new entries
        u32 new_space    = 256 * sizeof(queue_entry);
        if(q->entries == NULL)
        {
            init_mem_arena(&q->backing_mem, 128 * 1024);
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
    e->p = p;
    q->num_entries += 1;
    q->end += 1;
}

map_node *pop_from_queue(search_queue *q)
{
    queue_entry *e = &q->entries[0];
    map_node    *n = e->n;

    q->num_entries -= 1;
    q->end         -= 1;
    u32 mov_size    = q->num_entries * sizeof(queue_entry);
    memmove(e, e+1, mov_size);

    return n;
}

typedef struct
{
    u64 x;
    u64 y;
} byte_pos;

enum
{
    NONE,
    START,
    END,
    SAFE,
    CORRUPTED,
    COUNT,
};

void print_map(map_node *map, u64 map_w, u64 map_h)
{
    char map_line[256] = {0};
    for(u64 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u64 x = 0; x < map_w; x += 1, i += 1)
        {
            char c = map[i].type;
            //if(map[i].searched) c = 'O';
            //else
            {
                switch(c)
                {
                    case SAFE:      c = '.'; break;
                    case CORRUPTED: c = '#'; break;
                    case START:     c = 'S'; break;
                    case END:       c = 'E'; break;
                }
            }
            map_line[x] = c;
        }
        printf("%.*s\n", map_w, map_line);
    }
}

byte_pos *parse_positions(char *in, u64 in_len, u64 *ret_num_pos)
{
    u64 num_pos = 0;

    tokeniser t = init_tokeniser(in, in_len);
    for(token tok = read_token(&t); tok.type != TOKEN_END; tok = read_token(&t))
    {
        if(tok.type == TOKEN_NEWLINE) num_pos += 1;
    }

    byte_pos *pos = (byte_pos*)alloc(num_pos * sizeof(byte_pos));

    t = init_tokeniser(in, in_len);
    for(u64 i = 0; i < num_pos; i += 1)
    {
        byte_pos *dst = &pos[i];
        token tok = read_token(&t);
        dst->x = tok.int_val + 1;
        tok = read_token(&t);
        tok = read_token(&t);
        dst->y = tok.int_val + 1;
        tok = read_token(&t);
    }

    *ret_num_pos = num_pos;
    return pos;
}

map_node *get_node(map_node *map, u64 w, u64 x, u64 y)
{
    return &map[w*y + x];
}

void pathfind(map_node *map, u64 map_w, u64 map_h)
{
    search_queue q = {0};
    push_to_queue(&q, get_node(map, map_w, 1, 1), 0);
    do
    {
        map_node *n = pop_from_queue(&q);
        n->searched = 1;
        map_node *ma[4];
        ma[0] = get_node(map, map_w, n->x-1, n->y); //Left
        ma[1] = get_node(map, map_w, n->x+1, n->y); //Right
        ma[2] = get_node(map, map_w, n->x, n->y-1); //Up
        ma[3] = get_node(map, map_w, n->x, n->y+1); //Down

        for(u8 i = 0; i < 4; i += 1)
        {
            map_node *m = ma[i];
            if(m->type != CORRUPTED)
            {
                u64 new_cost = n->cost + 1;
                if(m->cost == 0 || m->cost > new_cost)
                {
                    m->cost = new_cost;
                    push_to_queue(&q, m, new_cost);
                }
            }
        }
    }
    while(!is_empty(&q));

}

void corrupt_byte(map_node *map, u64 w, byte_pos *p)
{
    u64 n       = p->y*w + p->x;
    map[n].type = CORRUPTED;
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = read_file_contents("aoc_18.txt", &in_file_size);
    u64   in_map_w = 71;
    u64   in_map_h = 71;
    u64   num_bytes_fall = 1024;
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
    u64   in_map_w = 7;
    u64   in_map_h = 7;
    u64   num_bytes_fall = 12;
#endif

    u64 num_positions = 0;
    byte_pos *positions = parse_positions(in_file_contents, in_file_size, &num_positions);
    printf("Num pos = %lu\n", num_positions);

    u64 map_w = in_map_w + 2;
    u64 map_h = in_map_h + 2;
    u64 map_size = map_w * map_h;
    map_node *map = (map_node*)alloc(map_size*sizeof(map_node));
    for(u64 y = 0, i = 0; y < map_h; y += 1)
    {
        for(u64 x = 0; x < map_w; x += 1, i += 1)
        {
            if(x == 0 || y == 0 || x == map_w -1 || y == map_h -1)
            map[i].type = CORRUPTED;
            else
            map[i].type = SAFE;

            map[i].x    = (s64)x;
            map[i].y    = (s64)y;
            map[i].cost = 0;
            map[i].searched = 0;
        }
    }
    map_node *start = get_node(map, map_w, 1, 1);
    start->type     = START;
    map_node *end   = get_node(map, map_w, map_w-2, map_h-2);
    end->type       = END;

    for(u64 i = 0; i < num_bytes_fall; i += 1)
    {
        corrupt_byte(map, map_w, &positions[i]);
    }

    pathfind(map, map_w, map_h);
    printf("Part 1: Num steps = %lu\n", get_node(map, map_w, map_w-2, map_h-2)->cost);

    for(u64 i = num_bytes_fall; i < num_positions; i += 1)
    {
        for(u64 j = 0; j < map_size; j += 1)
        {
            map[j].searched = 0;
            map[j].cost     = 0;
        }
        corrupt_byte(map, map_w, &positions[i]);
        pathfind(map, map_w, map_h);
        if(end->searched == 0)
        {
            printf("Part 2: First byte = %lu %lu\n", positions[i].x, positions[i].y);
            break;
        }
    }
    printf("DONE\n");
    return 0;
}
