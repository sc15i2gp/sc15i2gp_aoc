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

const char in_arg[] =
#if 0
"RL\n"
"\n"
"AAA = (BBB, CCC)\n"
"BBB = (DDD, EEE)\n"
"CCC = (ZZZ, GGG)\n"
"DDD = (DDD, DDD)\n"
"EEE = (EEE, EEE)\n"
"GGG = (GGG, GGG)\n";
"ZZZ = (ZZZ, ZZZ)\n"
#else
#if 0
"LR\n"
"\n"
"AAA = (AAB, XXX)\n"
"AAB = (XXX, ZZZ)\n"
"ZZZ = (AAB, XXX)\n"
"22A = (22B, XXX)\n"
"22B = (22C, 22C)\n"
"22C = (22Z, 22Z)\n"
"22Z = (22B, 22B)\n"
"XXX = (XXX, XXX)\n";
#else
#include "aoc_2023_8.txt"
#endif
#endif

typedef u64 node[2];
typedef struct
{
    char name[3];
} name_node;

u64 name_cmp(const char *n0, const char *n1)
{
    for(u64 i = 0; i < 3; ++i)
    {
        if(n0[i] != n1[i]) return 0;
    }
    return 1;
}

u64 find_name(name_node *name_table, u64 num_names, char *name)
{
    for(u64 i = 0; i < num_names; ++i)
    {
        if(name_cmp(name_table[i].name, name)) return i;
    }
    return -1;
}

void copy_name(char *dst, const char *src)
{
    for(u64 i = 0; i < 3; ++i) dst[i] = src[i];
}

void print_name(name_node *name_table, u64 n)
{
    printf("%.3s", name_table[n].name);
}

u64 gcd(u64 a, u64 b)
{
    while(a != b)
    {
        if(a > b)
        {
            a -= b;
        }
        else
        {
            b -= a;
        }
    }
    return a;
}

u64 lcm(u64 a, u64 b)
{
    return (a * b)/gcd(a, b);
}

//Nodes list (input) {u64, u64}
//Name table (from input) {name}
//Nodes to track (one or more) {u64...}

//Keep 0th node AAA
//Keep last node ZZZ
//0 for L, 1 for R
int main()
{
    u64 in_len = sizeof(in_arg) - 1;
    u64 num_nodes = 0;
    const char *in_end = in_arg + in_len;

    //Count number of nodes
    const char *c = find_next_char(in_arg, '\n');
    c += 2; //Skip to nodes list
    for(; c < in_end; ++num_nodes)
    {
        c = find_next_char(c, '\n');
        c += 1;
    }
    printf("Nodes = %u\n", num_nodes);

    //Count directions
    c = find_next_char(in_arg, '\n');
    u64 num_dirs = c - in_arg;
    printf("Num dirs = %u\n", num_dirs);

    u64 *dirs = VirtualAlloc(NULL, num_dirs * sizeof(u64), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    c = in_arg;
    for(u64 i = 0; i < num_dirs; ++i)
    {
        if(c[i] == 'L') dirs[i] = 0;
        if(c[i] == 'R') dirs[i] = 1;
    }

    //Allocate name table and add node names
    name_node *name_table = VirtualAlloc(NULL, num_nodes * sizeof(name_node), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    c = find_next_char(in_arg, '\n');
    c += 2;

    //Explicitly add AAA and ZZZ to name table
    copy_name(name_table[0].name,           "AAA");
    copy_name(name_table[num_nodes-1].name, "ZZZ");
    for(u64 i = 1; i < num_nodes - 1; ++i)
    {
        if(name_cmp(c, "AAA") || name_cmp(c, "ZZZ"))
        {
            i -= 1;
        }
        else
        {
            copy_name(name_table[i].name, c);
        }
        c = find_next_char(c, '\n');
        c += 1;
    }

    //Print name table
    printf("Name table:\n");
    for(u64 i = 0; i < num_nodes; ++i)
    {
        printf("%u: %.3s\n", i, name_table[i].name);
    }
    printf("\n");

    //Go through right names and add to nodes list
    node *nodes = VirtualAlloc(NULL, num_nodes * sizeof(node), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    c = find_next_char(in_arg, '\n');
    c += 2;
    for(u64 i = 0; i < num_nodes; ++i)
    {
        char name[3] = {};
        copy_name(name, c);
        u64 node_pos = find_name(name_table, num_nodes, name);
        
        c = find_next_char(c, '(');
        c += 1;
        copy_name(name, c);
        u64 l_pos = find_name(name_table, num_nodes, name);

        c = find_next_space(c);
        c = find_next_non_space(c);
        copy_name(name, c);
        u64 r_pos = find_name(name_table, num_nodes, name);

        nodes[node_pos][0] = l_pos;
        nodes[node_pos][1] = r_pos;

        c = find_next_char(c, '\n');
        c += 1;
    }

    //Print original input
    for(u64 i = 0; i < num_dirs; ++i)
    {
        if(dirs[i] == 0) printf("L");
        if(dirs[i] == 1) printf("R");
    }
    printf("\n");
    for(u64 i = 0; i < num_nodes; ++i)
    {
        print_name(name_table, i);
        printf(" = (");
        print_name(name_table, nodes[i][0]);
        printf(", ");
        print_name(name_table, nodes[i][1]);
        printf(")\n");
    }

    //u64 num_track_nodes = 1;
    //u64 track_nodes[] = {0};
    u64 steps = 1;

    u64 num_track_nodes = 0;
    for(u64 i = 0; i < num_nodes; ++i)
    {
        if(name_table[i].name[2] == 'A') ++num_track_nodes;
    }
    u64 nt = 0;
    u64 *track_nodes = VirtualAlloc(NULL, num_track_nodes * sizeof(u64), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    for(u64 i = 0; i < num_nodes; ++i)
    {
        if(name_table[i].name[2] == 'A')
        {
            track_nodes[nt] = i;
            nt += 1;
        }
    }

    for(u64 i = 0; i < num_track_nodes; ++i)
    {
        u64 d = 0;
        u64 steps = 1;
        for(;;++steps)
        {
            track_nodes[i] = nodes[track_nodes[i]][dirs[d]];
            if(name_table[track_nodes[i]].name[2] == 'Z') break;
            d += 1;
            if(d == num_dirs) d = 0;
        }
        printf("END: ");
        print_name(name_table, track_nodes[i]);
        printf("\n");
        track_nodes[i] = steps;
    }
    for(u64 i = 0; i < num_track_nodes; ++i)
    {
        printf("%u ", track_nodes[i]);
    }
    printf("\n");

    u64 l = lcm(track_nodes[0], track_nodes[1]);
    for(u64 i = 2; i < num_track_nodes; ++i)
    {
        l = lcm(l, track_nodes[i]);
    }
    printf("%lu\n", l);
    return 0;
}
