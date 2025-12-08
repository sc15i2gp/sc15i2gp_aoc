#include "util.h"

char start = 'S';
char split = '^';
char beam  = '|';
char empty = '.';

// Y is top-to-bottom
typedef struct
{
    u32 w, h;
    u32 sx, sy;
    char *contents;
} board;

void print_board(board *b)
{
    printf("Board w = %lu h = %lu sx = %lu sy = %lu\n", b->w, b->h, b->sx, b->sy);
    for(u32 y = 0, x = 0; y < b->h; y += 1, x += b->w)
    {
        printf("%.*s\n", b->w, &b->contents[x]);
    }
}

void set_space(board *b, u32 x, u32 y, char c)
{
    char *l = &b->contents[y*b->w + x];
    *l = c;
}

char get_space(board *b, u32 x, u32 y)
{
    return b->contents[y*b->w + x];
}

board read_input(char *in, u32 in_len)
{
    u32 sx = 0, sy = 0;
    u32 w = 0;
    for(; in[w] != '\n'; w += 1) if(in[w] == start) sx = w;
    u32 h = 0;
    for(u32 i = 0; i < in_len; i += 1) if(in[i] == '\n') h += 1;

    char *contents = alloc(w*h);
    for(u32 i = 0, j = 0; i < in_len; i += 1, j += 1)
    {
        if(in[i] == '\n') i += 1;
        contents[j] = in[i];
    }

    board b =
    {
        .w = w, .h = h, .sx = sx, .sy = sy,
        .contents = contents
    };
    return b;
}

u64 run_beam(board *b)
{
    u64 num_split = 0;
    set_space(b, b->sx, 1, beam);
    // Start at y = 2 since beam only goes downwards and we explicitly set the beam below the start
    for(u32 y = 2; y < b->h; y += 1)
    {
        for(u32 x = 0; x < b->w; x += 1)
        {
            char c = get_space(b, x, y);   // Current
            char d = get_space(b, x, y-1); // Above
            if(c == empty && d == beam) set_space(b, x, y, beam);
            if(c == split && d == beam)
            {
                num_split += 1;
                set_space(b, x-1, y, beam);
                set_space(b, x+1, y, beam);
            }
        }
    }
    return num_split;
}

u8 follow_beam(board *b, u32 ox, u32 oy, u32 *rx, u32 *ry)
{
    u32 x = ox, y = oy;
    u8 split = 0;
    for(char c = get_space(b, x, y); c == beam && y < b->h; c = get_space(b, x, y)) y += 1;
    split = y != b->h;

    *rx = x, *ry = y;
    return split;
}

typedef struct splt_node splt_node;

struct splt_node
{
    splt_node *l_child;
    splt_node *r_child;
    u64 val;
    u32 x, y;
    u8 visited;
};

typedef struct
{
    u32 node_cap;
    u32 nodes_used;
    splt_node **base;
    splt_node **top;
    splt_node **front;
    splt_node **back;
} splt_node_queue;

splt_node_queue alloc_queue(u32 cap)
{
    splt_node **base = alloc(cap * sizeof(splt_node*));
    splt_node_queue q =
    {
        .node_cap = cap, .nodes_used = 0,
        .base = base, .front = base, .back = base,
        .top = base + cap
    };
    return q;
}

u8 is_in_queue(splt_node_queue *q, splt_node *n)
{
    splt_node **p = q->front;
    for(u32 i = 0; i < q->nodes_used; i += 1)
    {
        if(*p == n) return 1;
        if(p == q->top) p = q->base;
        else p += 1;
    }
    return 0;
}

void push_to_queue(splt_node_queue *q, splt_node *n)
{
    if(!is_in_queue(q, n))
    {
        *q->back = n;
        if(q->back == q->top) q->back = q->base;
        else q->back += 1;
        q->nodes_used += 1;
    }
}

splt_node *pop_from_queue(splt_node_queue *q)
{
    splt_node *n = *q->front;
    if(q->front == q->top) q->front = q->base;
    else q->front += 1;
    q->nodes_used -= 1;
    return n;
}

u8 is_queue_empty(splt_node_queue *q)
{
    return q->front == q->back;
}

typedef struct
{
    u32 node_cap;
    splt_node **base;
    splt_node **top;
} splt_node_stack;

splt_node_stack alloc_stack(u32 cap)
{
    splt_node **base = alloc(cap * sizeof(splt_node*));
    splt_node_stack s = { .node_cap = cap, .base = base, .top = base };
    return s;
}

void push_to_stack(splt_node_stack *s, splt_node *n)
{
    *s->top = n;
    s->top += 1;
}

splt_node *pop_from_stack(splt_node_stack *s)
{
    s->top -= 1;
    splt_node *n = *s->top;
    return n;
}

u8 is_stack_empty(splt_node_stack *s)
{
    return s->base == s->top;
}

typedef struct
{
    u32 node_cap;
    u32 nodes_allocd;
    splt_node *nodes;
    splt_node *root;
} split_tree;

split_tree alloc_tree(u32 cap)
{
    splt_node *nodes = alloc(cap * sizeof(splt_node));
    split_tree t =
    {
        .node_cap = cap, .nodes_allocd = 0,
        .nodes = nodes, .root = nodes,
    };
    return t;
}

splt_node *push_node(split_tree *t, u32 x, u32 y)
{
    splt_node *n;
    u8 in_tree = 0;
    for(u32 i = 0; i < t->nodes_allocd; i += 1)
    {
        n = &t->nodes[i];
        if(n->x == x && n->y == y) { in_tree = 1; break; }
    }
    if(!in_tree)
    {
        n = &t->nodes[t->nodes_allocd];
        t->nodes_allocd += 1;
        n->x = x; n->y = y;
    }
    return n;
}

split_tree read_tree(board *b)
{
    splt_node_queue q = alloc_queue(2000);
    split_tree t = alloc_tree(2000);
    u32 x = b->sx, y = 1;
    follow_beam(b, x, y, &x, &y);
    t.root = push_node(&t, x, y);
    for(push_to_queue(&q, t.root); !is_queue_empty(&q);)
    {
        splt_node *n = pop_from_queue(&q);
        u32 dx, dy;
        x = n->x; y = n->y;
        u8 l_split = follow_beam(b, x-1, y, &dx, &dy);
        if(l_split)
        {
            splt_node *o = push_node(&t, dx, dy);
            n->l_child = o;
            push_to_queue(&q, o);
        }
        u8 r_split = follow_beam(b, x+1, y, &dx, &dy);
        if(r_split)
        {
            splt_node *o = push_node(&t, dx, dy);
            n->r_child = o;
            push_to_queue(&q, o);
        }
    }

    return t;
}

u64 count_node_paths(splt_node *n)
{
    if(n->val == 0)
    {
        if(n->l_child) n->val += count_node_paths(n->l_child); else n->val += 1;
        if(n->r_child) n->val += count_node_paths(n->r_child); else n->val += 1;
    }
    return n->val;
}

u64 count_paths(split_tree *t)
{
    return count_node_paths(t->root);
}

char example[] =
{
    ".......S.......\n"
    "...............\n"
    ".......^.......\n"
    "...............\n"
    "......^.^......\n"
    "...............\n"
    ".....^.^.^.....\n"
    "...............\n"
    "....^.^...^....\n"
    "...............\n"
    "...^.^...^.^...\n"
    "...............\n"
    "..^...^.....^..\n"
    "...............\n"
    ".^.^.^.^.^...^.\n"
    "...............\n"
};

int main()
{
#if 0
    const char *in = example;
    u32 in_len = strlen(in);
#else
    u64 in_len;
    char *in = read_file_contents("aoc_7.txt", &in_len);
#endif
    board b = read_input(in, in_len);
    u64 n = run_beam(&b);
    printf("Num split = %llu\n", n);
    //print_board(&b);
    split_tree t = read_tree(&b);
    n = count_paths(&t);
    printf("Paths = %llu\n", n);
    printf("DONE!\n");
    return 0;
}
