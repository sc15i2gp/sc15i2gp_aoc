#include "util.h"
#include <string.h>

const char test[] =
"47|53\n"
"97|13\n"
"97|61\n"
"97|47\n"
"75|29\n"
"61|13\n"
"75|53\n"
"29|13\n"
"97|29\n"
"53|29\n"
"61|53\n"
"97|53\n"
"61|29\n"
"47|13\n"
"75|47\n"
"97|75\n"
"47|61\n"
"75|61\n"
"47|29\n"
"75|13\n"
"53|13\n"
"\n"
"75,47,61,53,29\n"
"97,61,53,29,13\n"
"75,29,13\n"
"75,97,47,61,53\n"
"61,13,29\n"
"97,13,75,29,47\n";

typedef struct
{
    u32  num_orderings;
    u32 *orderings;

    u32  num_updates;
    u8  *update_correct;
    u32 *update_lens;
    u32 *updates;
} page_updates;

void print_page_updates(page_updates updates)
{
    for(u32 i = 0; i < updates.num_orderings; i += 1)
    {
        printf("%u|%u\n", updates.orderings[2*i], updates.orderings[2*i + 1]);
    }
    printf("\n");
    u32 update_offset = 0;
    for(u32 i = 0; i < updates.num_updates; i += 1)
    {
        u32 update_len = updates.update_lens[i];
        printf("%u:", updates.update_correct[i]);
        for(u32 j = 0; j < update_len; j += 1)
        {
            char end_c = (j < update_len - 1) ? ',' : '\n';
            printf("%u%c", updates.updates[update_offset+j], end_c);
        }
        update_offset += update_len;
    }
}

u32 find_in_list(u32 to_find, u32 *list, u32 list_len)
{
    for(u32 i = 0; i < list_len; i += 1)
    {
        if(list[i] == to_find) return i;
    }

    return -1;
}

void sort_update(u32 *update, u32 update_len, u32 *orderings, u32 num_orderings)
{
    for(u32 i = 0; i < update_len; i += 1)
    {
        for(u32 j = i; j < update_len; j += 1)
        {
            u32 ui = update[i];
            u32 uj = update[j];

            for(u32 k = 0; k < num_orderings; k += 1)
            {
                if(orderings[2*k] == uj && orderings[2*k+1] == ui)
                {
                    update[i] = uj;
                    update[j] = ui;
                    break;
                }
            }
        }
    }
}

page_updates parse_page_updates(char *updates_file, u32 updates_file_size)
{
    tokeniser t = init_tokeniser(updates_file, updates_file_size);

    u32 num_orderings = 0;
    u32 num_updates   = 0;
    u32 num_pages     = 0;
    //Count number of orderings and updates
    for(token tok = read_token(&t); tok.type != TOKEN_END; tok = read_token(&t))
    {
        if(tok.type == TOKEN_PIPE) num_orderings += 1;
        if(tok.type == TOKEN_COMMA)
        {
            num_updates += 1;
            num_pages   += 1;
            for(; tok.type != TOKEN_NEWLINE && tok.type != TOKEN_END; tok = read_token(&t))
            {
                if(tok.type == TOKEN_INTEGER) num_pages += 1;
            }
        }
    }

    page_updates updates   = {0};
    updates.num_orderings  = num_orderings;
    updates.num_updates    = num_updates;
    updates.orderings      = (u32*)alloc(2*num_orderings*sizeof(u32));
    updates.update_lens    = (u32*)alloc(num_updates*sizeof(u32));
    updates.update_correct = (u8*)alloc(num_updates*sizeof(u8));
    updates.updates        = (u32*)alloc(num_pages*sizeof(u32));

    t = init_tokeniser(updates_file, updates_file_size);
    for(u32 i = 0; i < num_orderings; i += 1)
    {
        u32 *dst = &updates.orderings[2*i];
        token tok = read_token(&t); //INTEGER
        dst[0] = tok.int_val;
        tok = read_token(&t); //PIPE
        tok = read_token(&t); //INTEGER
        dst[1] = tok.int_val;
        tok = read_token(&t); //NEWLINE
    }
    read_token(&t); //Skip newline separating orderings and updates
    
    u32 update_offset = 0;
    for(u32 i = 0; i < num_updates; i += 1)
    {
        updates.update_correct[i] = 1;
        u32 update_len = 0;
        for(token tok = read_token(&t); tok.type != TOKEN_NEWLINE; tok = read_token(&t))
        {
            if(tok.type == TOKEN_INTEGER)
            {
                updates.updates[update_offset + update_len] = tok.int_val;
                update_len += 1;
            }
        }
        updates.update_lens[i] = update_len;
        update_offset += update_len;
    }

    return updates;
}

int main()
{
    u32 in_file_size;
    char *in_file_contents = (char*)read_file_contents("aoc_5.txt", &in_file_size);
    page_updates updates = parse_page_updates(in_file_contents, in_file_size);

    for(u32 i = 0; i < updates.num_orderings; i += 1)
    {
        u32 *ord = &updates.orderings[2*i];
        u32 update_offset = 0;
        for(u32 j = 0; j < updates.num_updates; j += 1)
        {
            u32 *update = &updates.updates[update_offset];
            u32  up_len = updates.update_lens[j];
            u32  o0     = find_in_list(ord[0], update, up_len);
            u32  o1     = find_in_list(ord[1], update, up_len);

            if((o0 != -1 && o1 != -1) && (o1 < o0))
            {
                updates.update_correct[j] = 0;
            }

            update_offset += up_len;
        }
    }
    u32 middle_sum = 0;
    u32 update_offset = 0;
    for(u32 i = 0; i < updates.num_updates; i += 1)
    {
        u32 *update = &updates.updates[update_offset];
        u32  up_len = updates.update_lens[i];
        if(updates.update_correct[i])
        {
            middle_sum += update[up_len/2];
        }

        update_offset += up_len;
    }

    printf("Part 1: Sum of update middles = %u\n", middle_sum);

    update_offset = 0;
    for(u32 i = 0; i < updates.num_updates; i += 1)
    {
        u32 *update = &updates.updates[update_offset];
        u32  up_len = updates.update_lens[i];
        if(!updates.update_correct[i])
        {
            sort_update(update, up_len, updates.orderings, updates.num_orderings);
            updates.update_correct[i] = 2;
        }
        update_offset += up_len;
    }

    middle_sum = 0;
    update_offset = 0;
    for(u32 i = 0; i < updates.num_updates; i += 1)
    {
        u32 *update = &updates.updates[update_offset];
        u32  up_len = updates.update_lens[i];
        if(updates.update_correct[i] == 2)
        {
            middle_sum += update[up_len/2];
        }

        update_offset += up_len;
    }
    printf("Part 2: Sum of update middles = %u\n", middle_sum);
    return 0;
}
