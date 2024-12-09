#include "util.h"

char test[] =
"2333133121414131402\n";

void print_fs(s64 *fs, u64 fs_len)
{
    for(u64 i = 0; i < fs_len; i += 1)
    {
        s64 block = fs[i];
        if(block >= 0) printf("%d", block);
        else           printf(".");
    }
}

void swap(s64 *list, u64 s0, u64 s1)
{
    s64 tmp  = list[s0];
    list[s0] = list[s1];
    list[s1] = tmp;
}

s64 checksum(s64 *fs, u64 fs_len)
{
    s64 cs = 0;
    for(u64 i = 0; i < fs_len; i += 1)
    {
        s64 f = fs[i];
        if(f >= 0)
        {
            cs += f * (s64)(i);
        }
    }
    return cs;
}

typedef struct free_block free_block;

struct free_block
{
    u64 start;
    u64 size;
    free_block *prev;
    free_block *next;
};

free_block *find_first_block_of_size(free_block *free_blocks, u64 start, u64 size)
{
    free_block **blk;
    for(blk = &free_blocks; *blk != NULL && (*blk)->size < size; blk = &((*blk)->next));
    if(*blk && (*blk)->start > start) return NULL;
    return *blk;
}

void remove_block(free_block *blk)
{
    if(blk->prev)
    {
        blk->prev->next = blk->next;
    }
    if(blk->next)
    {
        blk->next->prev = blk->prev;
    }
}

void fill_fs(s64 *fs, u64 fs_len, char *map, u64 map_len)
{
    u8 file = 1;
    s64 file_id = 0;
    for(u64 i = 0, j = 0; i < map_len; i += 1)
    {
        u32  n = (u32)(map[i] - '0');
        s64 id = (file) ? file_id : -1;

        for(u32 k = 0; k < n; k += 1, j += 1)
        {
            fs[j] = id;
        }
        if(file) file_id += 1;
        file = !file;
    }
    
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = (char*)read_file_contents("aoc_9.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(in_file_contents);
#endif

    u64 fs_len = 0;
    for(u64 i = 0; i < in_file_size-1; i += 1) 
    {
        fs_len += (in_file_contents[i] - '0');
    }

    s64 *fs = (s64*)alloc(fs_len * sizeof(s64));

    fill_fs(fs, fs_len, in_file_contents, in_file_size - 1);

    u64 left_free  = 0;
    u64 right_full = fs_len - 1;

    for(; fs[left_free]  != -1; left_free  += 1);
    for(; fs[right_full] == -1; right_full -= 1);

    while(left_free < right_full)
    {
        swap(fs, left_free, right_full);
        for(; left_free  < fs_len && fs[left_free]  != -1; left_free  += 1);
        for(; right_full > 0      && fs[right_full] == -1; right_full -= 1);
    }

    s64 cs = checksum(fs, fs_len);
    printf("Part 1: Checksum = %ld\n", cs);

    fill_fs(fs, fs_len, in_file_contents, in_file_size-1);

    u64 num_free_blocks = 0;
    for(u64 i = 0; i < fs_len; i += 1)
    {
        if(fs[i] == -1)
        {
            num_free_blocks += 1;
            for(; fs[i] == -1; i += 1);
        }
    }

    free_block *free_blocks = (free_block*)alloc(num_free_blocks * sizeof(free_block));
    
    u64 free_block_index   = 0;
    free_block *prev_block = NULL;
    for(u64 i = 0; i < fs_len; i += 1)
    {
        if(fs[i] == -1)
        {
            free_block *block = &free_blocks[free_block_index];
            block->start = i;
            block->prev  = prev_block;
            
            for(; fs[i] == -1; i += 1);

            block->size = i - block->start;

            if(block->prev) block->prev->next = block;

            prev_block = block;
            free_block_index  += 1;
        }
    }

    for(s64 i = fs_len - 1; i > 0; i -= 1)
    {
        u64 blk_end = i;
        u64 blk_len = 1;
        s64 blk_id  = fs[i];
        if(blk_id == -1) continue;
        for(; i > 0 && fs[i-1] == blk_id; i -= 1, blk_len += 1);
        u64 blk_start = i;
        
        free_block *free_blk = find_first_block_of_size(free_blocks, blk_start, blk_len);
        if(free_blk)
        {
            for(u64 j = 0; j < blk_len; j += 1)
            {
                swap(fs, blk_start+j, free_blk->start+j);
            }
            free_blk->start += blk_len;
            free_blk->size  -= blk_len;
            if(free_blk->size == 0)
            {
                remove_block(free_blk);
            }
        }
    }

    cs = checksum(fs, fs_len);
    printf("Part 2: Checksum = %ld\n", cs);
	return 0;
}
