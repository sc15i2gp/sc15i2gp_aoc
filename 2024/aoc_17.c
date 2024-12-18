#include "util.h"

char test[] =
"Register A: 729\n"
"Register B: 0\n"
"Register C: 0\n"
"\n"
"Program: 0,1,5,4,3,0\n";

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
    u64 a;
    u32 p;
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

void push_to_queue(search_queue *q, u64 a, u32 p)
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

    e->a = a;
    e->p = p;
    q->num_entries += 1;
    q->end += 1;
}

u64 pop_from_queue(search_queue *q)
{
    queue_entry *e = &q->entries[0];
    u64          a = e->a;

    q->num_entries -= 1;
    q->end         -= 1;
    u32 mov_size    = q->num_entries * sizeof(queue_entry);
    memmove(e, e+1, mov_size);

    return a;
}

typedef struct
{
    u64 ra;
    u64 rb;
    u64 rc;
    u64 out;
    u8 pc;
    u8  mem_len;
    u8  mem[256];
}   machine;

enum
{
    NONE,
    HALT,
    CONT,
    XOR,
    MOD,
    DIV,
    OUTPUT,
    PASS,
    FAIL,
    COUNT,
};

void print_machine(machine *m)
{
    printf("Register A: %lu\n", m->ra);
    printf("Register B: %lu\n", m->rb);
    printf("Register C: %lu\n", m->rc);
    printf("PC:         %u\n", m->pc);
    printf("OUT:        %lu\n", m->out);
    printf("PROGRAM: ");
    for(u64 i = 0; i < m->mem_len; i += 1) printf("%u,", m->mem[i]);
    printf("\n");
}

void alu(u64 *dst, u64 *src0, u64 *src1, u8 op)
{
    switch(op)
    {
        case XOR:
        {
            *dst = *src0 ^ *src1;
            break;
        }
        case MOD:
        {
            *dst = *src0 & 0x7;
            break;
        }
        case DIV:
        {
            u64 n = *src0;
            u64 d = *src1;
            *dst = n >> d;
            break;
        }
    }
}

u64 decode_operand(machine *cpu, u8 op)
{
    switch(op)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        {
            return (u64)op;
        }
        case 4: return cpu->ra;
        case 5: return cpu->rb;
        case 6: return cpu->rc;
        case 7: //TODO: Halt
    }
}

u8 run_instr(machine *cpu, u8 *output)
{
    //Get instruction
    if(cpu->pc >= cpu->mem_len) return HALT;
    u8 instr = cpu->mem[cpu->pc];
    u8 op    = cpu->mem[cpu->pc+1];

    //Decode instruction
    u8   jump   = 0;
    u8   out    = 0;
    u8   alu_op = NONE;
    u64  alu_lit = 0;
    u64 *alu_dst = NULL, *alu_src0 = NULL, *alu_src1 = NULL;
    switch(instr)
    {
        case 0:
        {
            alu_op  = DIV;
            alu_dst = &cpu->ra;
            alu_src0 = &cpu->ra;
            alu_lit  = decode_operand(cpu, op);
            alu_src1 = &alu_lit;
            break;
        }
        case 1:
        {
            alu_op = XOR;
            alu_dst = &cpu->rb;
            alu_src0 = &cpu->rb;
            alu_lit  = op;
            alu_src1 = &alu_lit;
            break;
        }
        case 2:
        {
            alu_op = MOD;
            alu_dst = &cpu->rb;
            alu_lit = decode_operand(cpu, op);
            alu_src0 = &alu_lit;
            break;
        }
        case 3:
        {
            jump = 1;
            break;
        }
        case 4:
        {
            alu_op = XOR;
            alu_dst = &cpu->rb;
            alu_src0 = &cpu->rb;
            alu_src1 = &cpu->rc;
            break;
        }
        case 5:
        {
            out    = 1;
            alu_op = MOD;
            alu_dst = &cpu->out;
            alu_lit = decode_operand(cpu, op);
            alu_src0 = &alu_lit;
            break;
        }
        case 6:
        {
            alu_op = DIV;
            alu_dst = &cpu->rb;
            alu_src0 = &cpu->ra;
            alu_lit = decode_operand(cpu, op);
            alu_src1 = &alu_lit;
            break;
        }
        case 7:
        {            
            alu_op = DIV;
            alu_dst = &cpu->rc;
            alu_src0 = &cpu->ra;
            alu_lit = decode_operand(cpu, op);
            alu_src1 = &alu_lit;
            break;
        }
    }

    u8 state = CONT;
    if(alu_op != NONE) alu(alu_dst, alu_src0, alu_src1, alu_op);
    if(out) {*output = (u8)cpu->out; state = OUTPUT;}
    if(jump && cpu->ra) cpu->pc = op - 2;

    cpu->pc += 2;
    return state;
}

void parse_machine(char *in, u64 in_len, machine *cpu)
{
    cpu->mem_len = 0;
    cpu->pc      = 0;
    cpu->out     = 0;
    tokeniser t = init_tokeniser(in, in_len);

    token tok;
    for(u64 i = 0; i < 6; i += 1) tok = read_token(&t);
    cpu->ra = tok.int_val;
    for(u64 i = 0; i < 7; i += 1) tok = read_token(&t);
    cpu->rb = tok.int_val;
    for(u64 i = 0; i < 7; i += 1) tok = read_token(&t);
    cpu->rc = tok.int_val;
    for(u64 i = 0; i < 5; i += 1) tok = read_token(&t);

    for(; tok.type != TOKEN_END && tok.type != TOKEN_NEWLINE;)
    {
        tok = read_token(&t);
        cpu->mem[cpu->mem_len] = (u8)tok.int_val;
        cpu->mem_len += 1;
        tok = read_token(&t);
    }
}

u8 run_program_check(machine *cpu, u8 *out, u64 *out_len, u64 *num_run, u8 n)
{
    *num_run = 0;
    *out_len = 0; 
    u8 state;
    do
    {
        state = run_instr(cpu, &out[*out_len]);
        *num_run += 1;
        if(state == OUTPUT) *out_len += 1;
    }
    while(state != HALT);

    if(n)
    {
        for(u64 i = 0; i < *out_len; i += 1)
        {
            if(out[i] != cpu->mem[cpu->mem_len - *out_len + i]) return 0;
        }
    }
    return 1;
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = read_file_contents("aoc_17.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
#endif

    machine cpu = {0};
    /*
    cpu.ra = 117440;
    cpu.mem[0] = 0; cpu.mem[1] = 3; cpu.mem[2] = 5; cpu.mem[3] = 4; cpu.mem[4] = 3; cpu.mem[5] = 0;
    cpu.mem_len = 6;
    */
    parse_machine(in_file_contents, in_file_size, &cpu);

    u64 out_max = 256;
    u64 num_run = 0;
    u64 out_len = 0;
    u8  *out = (u8*)alloc(out_max);

    print_machine(&cpu); printf("\n");
    run_program_check(&cpu, out, &out_len, &num_run, 0);
    printf("\n"); print_machine(&cpu); printf("\n");
    printf("Part 1: Output (in %u instrs): ", num_run);
    for(u64 i = 0; i < out_len; i += 1) printf("%u,", out[i]);
    printf("\n");

    u8  found = 0;
    u64 min   = 0;
    search_queue q = {0};
    push_to_queue(&q, 46, 0); //46 gives final 2 in program sequence: "3, 0,"
    do
    {
        u64 a = pop_from_queue(&q);
        for(u64 i = a*8; i < (a+1)*8; i += 1)
        {
            num_run = 0;
            out_len = 0;
            for(u64 j = 0; j < out_max; j += 1) out[j] = 0;
            cpu.ra  = i;
            cpu.rb  = 0;
            cpu.rc  = 0;
            cpu.out = 0;
            cpu.pc  = 0;

            u8 out_matches_program = run_program_check(&cpu, out, &out_len, &num_run, 1);
            if(out_matches_program)
            {
                push_to_queue(&q, i, 0);

                if(out_len == 16)
                {
                    min = i;
                    found = 1;
                    break;
                }
            }
        }
    }
    while(!is_empty(&q) && !found);
    printf("Part 2: Min = %lu\n", min);

    printf("\nDONE\n");
    return 0;
}
