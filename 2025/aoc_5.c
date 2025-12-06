#include "util.h"

u64 _min(u64 n, u64 m)
{
    if(n < m) return n; else return m;
}

u64 _max(u64 n, u64 m)
{
    if(n > m) return n; else return m;
}

typedef struct
{
    u64 min; u64 max;
} range;

void print_range(range *r, u32 i)
{
    printf("%lu: %llu-%llu", i, r->min, r->max);
}

void print_ranges(range *r, u32 n)
{
    for(u32 i = 0; i < n; i += 1) { print_range(&r[i], i); printf("\n"); }
}

u8 is_in_range(u64 n, range r)
{
    return r.min <= n && n <= r.max;
}

u8 can_be_merged(range r0, range r1)
{
    if(r1.min <= r0.min && r0.min <= r1.max ||
       r0.min <= r1.min && r1.min <= r0.max) return 1;
    else return 0;
}

range merge_ranges(range r0, range r1)
{
    range r;
    r.min = _min(r0.min, r1.min);
    r.max = _max(r0.max, r1.max);
    return r;
}

u64 range_size(range r)
{
    return r.max - r.min + 1;
}

typedef struct
{
    u32 num_ranges;
    u32 num_ids;
    range *ranges;
    u64 *ids;
} inventory;

void print_inventory(inventory *inv)
{
    print_ranges(inv->ranges, inv->num_ranges);
    printf("\n");
    for(u32 i = 0; i < inv->num_ids; i += 1)
    {
        printf("%llu\n", inv->ids[i]);
    }
}

inventory read_input(char *in, u32 in_len)
{
    u32 num_ranges = 0;
    u32 i;
    for(i = 0; i < in_len; i += 1)
    {
        if(in[i] == '\n') { num_ranges += 1; if(in[i+1] == '\n') break; }
    }
    u32 num_ids = 0;
    for(i += 2;i < in_len; i += 1)
    {
        if(in[i] == '\n') num_ids += 1;
    }

    range *ranges = (range*)alloc(num_ranges*sizeof(range));
    u64 *ids = alloc(num_ids*sizeof(u64));
    tokeniser to = init_tokeniser(in, in_len);
    token t;
    for(u32 i = 0; i < num_ranges; i += 1)
    {
        t = read_token(&to); read_token(&to);
        ranges[i].min = (u64)t.int_val;
        t = read_token(&to); read_token(&to);
        ranges[i].max = (u64)t.int_val;
    }
    read_token(&to);
    for(u32 i = 0; i < num_ids; i += 1)
    {
        t = read_token(&to); read_token(&to);
        ids[i] = t.int_val;
    }

    inventory inv =
    {
        .num_ranges = num_ranges, .num_ids = num_ids,
        .ranges = ranges, .ids = ids,
    };
    return inv;
}

u32 merge_all_ranges(inventory *inv, inventory *new_inv)
{
    u32 num_merged = 0;
    new_inv->num_ranges = 0;
    for(u32 i = 0; i < inv->num_ranges; i += 1)
    {
        // Try to merge an old range into one of the new ranges
        // Otherwise push to list
        range *r = &inv->ranges[i];
        u8 merged = 0;
        for(u32 j = 0; j < new_inv->num_ranges; j += 1)
        {
            range *new_r = &new_inv->ranges[j];
            if(can_be_merged(*r, *new_r)) { *new_r = merge_ranges(*r, *new_r); merged = 1; }
        }
        if(!merged) { new_inv->ranges[new_inv->num_ranges] = *r; new_inv->num_ranges += 1; }
        else num_merged += 1;
    }
    return num_merged;
}

u32 num_fresh(inventory *inv)
{
    u32 n = 0;
    for(u32 i = 0; i < inv->num_ids; i += 1)
    {
        u64 id = inv->ids[i];
        for(u32 j = 0; j < inv->num_ranges; j += 1)
        {
            range r = inv->ranges[j];
            if(is_in_range(id, r)) n += 1;
        }
    }
    return n;
}

u64 total_fresh(inventory *inv)
{
    u64 n = 0;
    for(u32 i = 0; i < inv->num_ranges; i += 1) n += range_size(inv->ranges[i]);
    return n;
}

char example[] =
{
    "3-5\n"
    "10-14\n"
    "16-20\n"
    "12-18\n"
    "\n"
    "1\n"
    "5\n"
    "8\n"
    "11\n"
    "17\n"
    "32\n"
};

int main()
{
#if 0
    const char *_in = example;
    u32 in_len = strlen(_in);
    char *in = alloc(in_len);
#else
    u32 in_len;
    char *_in = read_file_contents("aoc_5.txt", &in_len);
    char *in = alloc(in_len);
#endif
    for(u32 i = 0; i < in_len; i += 1) { if(_in[i] == '-') in[i] = '|'; else in[i] = _in[i]; }
    inventory inv_old = read_input(in, in_len);
    printf("Inventory: %lu ranges %lu ids\n", inv_old.num_ranges, inv_old.num_ids);
    inventory inv_new =
    {
        .num_ranges = inv_old.num_ranges, .num_ids = inv_old.num_ids,
        .ids = inv_old.ids, .ranges = (range*)alloc(inv_old.num_ranges * sizeof(range))
    };
    inventory *inv = &inv_old;
    inventory *inv_1 = &inv_new;
    u32 num_merged = 0;
    do
    {
        num_merged = merge_all_ranges(inv, inv_1);
        inventory *tmp = inv; inv = inv_1; inv_1 = tmp;
    }
    while(num_merged > 0);
    printf("Num fresh = %lu\n", num_fresh(inv));
    printf("Total fresh = %llu\n", total_fresh(inv));
    printf("DONE!\n");
    return 0;
}
