#include "util.h"

typedef struct
{
    u32 num_operands;
    u8  operator;
    u64 *operands;
} problem;

u64 do_problem(problem *p)
{
    u64 answer;
    if(p->operator == '+') { answer = 0; for(u32 i = 0; i < p->num_operands; i += 1) answer += p->operands[i]; }
    else                   { answer = 1; for(u32 i = 0; i < p->num_operands; i += 1) answer *= p->operands[i]; }
    return answer;
}

void print_problem(problem *p)
{
    printf("%llu", p->operands[0]);
    for(u32 i = 1; i < p->num_operands; i += 1)
    {
        printf(" %c %llu", p->operator, p->operands[i]);
    }
}

void set_problem_operand(problem *ps, u32 problem, u32 operand, u64 val)
{
    ps[problem].operands[operand] = val;
}

u32 push_problem_operand(problem *p, u64 val)
{
    u32 n = p->num_operands;
    p->operands[n] = val;
    p->num_operands += 1;
    return n;
}

typedef struct
{
    u32 num_problems;
    problem *problems;
} problem_list;

void print_problem_list(problem_list *p)
{
    printf("Problem list: %lu problems\n", p->num_problems);
    for(u32 i = 0; i < p->num_problems; i += 1) { print_problem(&p->problems[i]); printf("\n"); }
}

u64 do_all_problems(problem_list *p)
{
    u64 answer_sum = 0;
    for(u32 i = 0; i < p->num_problems; i += 1)
    {
        answer_sum += do_problem(&p->problems[i]);
    }
    return answer_sum;
}

// First part of today's problem significantly easier to parse so doing it separately
problem_list read_input_0(char *in, u32 in_len)
{
    u32 num_problems = 0;
    for(u32 i = 0; in[i] != '\n'; num_problems += 1)
    {
        for(; is_num_char(in[i]); i += 1);  // Move to space
        for(; in[i] == ' '; i += 1); // Move to next number or \n
    }
    u32 num_ops = 0; // Same for every problem in part 1
    for(u32 i = 0; i < in_len; i += 1) if(in[i] == '\n') num_ops += 1;
    num_ops -= 1; // Dec since we're counting the operator row
    problem *problems = (problem*)alloc(num_problems * sizeof(problem));
    u64     *operands = (u64*)alloc(num_problems * num_ops * sizeof(u64));
    for(u32 i = 0, j = 0; i < num_problems; i += 1, j += num_ops)
    {
        problems[i].operands = &operands[j]; problems[i].num_operands = num_ops;
    }

    // Read problem operands and operators
    tokeniser to = init_tokeniser(in, in_len);
    for(u32 i = 0; i < num_ops; i += 1)
    {
        token t = lookahead_token(&to);
        if(t.type == TOKEN_SPACE) t = read_token(&to); // Skip space at start of line if there is one
        for(u32 j = 0; j < num_problems; j += 1)
        {
            t = read_token(&to); // Read number
            set_problem_operand(problems, j, i, (u64)t.int_val);
            t = read_token(&to); // Read space
        }
        if(t.type == TOKEN_SPACE) read_token(&to);
    }
    for(u32 i = 0; i < num_problems; i += 1)
    {
        token t = read_token(&to); read_token(&to); // Read operator and skip space
        problems[i].operator = *t.loc;
    }

    problem_list p = {.num_problems = num_problems, .problems = problems};
    return p;
}

problem_list read_input_1(char *in, u32 in_len)
{
    // Count row length since all text should be aligned
    u32 row_length;
    for(row_length = 0; in[row_length] != '\n'; row_length += 1);
    row_length += 1; // Account for newline at end
    // Count number of rows
    u32 num_rows = 0;
    for(u32 i = 0; i < in_len; i += row_length, num_rows += 1);
    // Find first operator symbol
    u32 first_op;
    for(first_op = 0; !(in[first_op] == '+' || in[first_op] == '*'); first_op += 1);
    // Count problems by number of operator symbols
    u32 num_problems = 0;
    for(u32 i = first_op; in[i] != '\n'; i += 1) { if(in[i] == '*' || in[i] == '+') num_problems += 1; }
    problem *problems = (problem*)alloc(num_problems * sizeof(problem));
    u32 max_operands = 0;
    // Read operators and count max number of operands per problem
    char *c = &in[first_op];
    for(u32 i = 0; i < num_problems; i += 1)
    {
        problems[i].operator = c[0];
        u32 num_cols_to_problem = 1;
        for(; c[num_cols_to_problem] == ' '; num_cols_to_problem += 1);
        c += num_cols_to_problem;
        problems[i].num_operands = num_cols_to_problem; // Just for storage, will replace later
        max_operands += num_cols_to_problem;
    }
    // Read operands
    u64 *operands = alloc(max_operands*sizeof(u64));
    char *operand = alloc(num_rows);
    u32 operands_read = 0;
    u32 cols_done = 0;
    for(u32 i = 0; i < num_problems; i += 1)
    {
        problem *prob = &problems[i];
        u32 num_cols = prob->num_operands;
        u64 *operand_loc = &operands[operands_read];
        prob->operands = operand_loc;
        prob->num_operands = 0;
        for(u32 col = 0; col < num_cols; col += 1, cols_done += 1)
        {
            char *col_loc = &in[cols_done];
            u32 operand_length = 0;
            for(u32 row = 0; row < num_rows-1; row += 1, col_loc += row_length)
            {
                char c = *col_loc;
                if(c != ' ') { operand[operand_length] = c; operand_length += 1; }
            }
            if(operand_length > 0)
            {
                operands_read += 1;
                u64 op_val = str_to_s64(operand, operand_length);
                u32 n = push_problem_operand(prob, op_val); 
            }
        }
    }
    problem_list p = {.num_problems = num_problems, .problems = problems };
    return p;
}

char example[] =
{
    "123 328  51 64 \n"
    " 45 64  387 23 \n"
    "  6 98  215 314\n"
    "*   +   *   +  \n"
};

int main()
{
#if 0
    const char *in = example;
    u32 in_len = strlen(in);
#else
    u32 in_len;
    char *in = read_file_contents("aoc_6.txt", &in_len);
#endif

    problem_list p = read_input_0(in, in_len);
    u64 answer_sum = do_all_problems(&p);
    printf("Answer sum = %llu\n", answer_sum);

    problem_list q = read_input_1(in, in_len);
    answer_sum = do_all_problems(&q);
    printf("Answer sum = %llu\n", answer_sum);

    printf("DONE!\n");
    return 0;
}
