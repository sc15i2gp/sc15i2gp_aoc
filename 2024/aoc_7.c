#include "util.h"

char test[] =
"190: 10 19\n"
"3267: 81 40 27\n"
"83: 17 5\n"
"156: 15 6\n"
"7290: 6 8 6 15\n"
"161011: 16 10 13\n"
"192: 17 8 14\n"
"21037: 9 7 18 13\n"
"292: 11 6 16 20\n";

typedef struct
{
    u64 num_equations;
    u8  *is_true;
    u64 *lens;
    u64 *operands;
} equation_list;

void print_equation_list(equation_list *equations)
{
    u64 offset = 0;
    for(u64 i = 0; i < equations->num_equations; i += 1)
    {
        u64  len = equations->lens[i];
        u64 *ops = &equations->operands[offset];
        printf("%u - %lu:", equations->is_true[i], ops[0]);
        for(u64 j = 1; j < len; j += 1)
        {
            printf(" %lu", ops[j]);
        }
        offset += len;
        printf("\n");
    }
}

void gen_operator_list(u8 *operators, u64 num_operators, u64 set_bits, u64 cat_bits)
{
    for(u64 i = 0; i < num_operators; i += 1)
    {
        u64 n = (set_bits >> i) & 0x1;
        u64 c = (cat_bits >> i) & 0x1;
        if(c)      operators[i] = '|';
        else if(n) operators[i] = '+';
        else       operators[i] = '*';
    }
}

u64 concat(u64 n, u64 m)
{
    u64 p = m;
    u64 num_m_digits = 0;
    do
    {
        p /= 10;
        num_m_digits += 1;
    }
    while(p > 0);

    for(u64 i = 0; i < num_m_digits; i += 1, n *= 10);
    return n + m;
}

u64 eval_equation(u64 *operands, u8 *operators, u64 num_operands)
{
    u64 answer = operands[0];
    for(u64 i = 1; i < num_operands; i += 1)
    {
        if(operators[i-1] == '+') answer += operands[i];
        if(operators[i-1] == '*') answer *= operands[i];
        if(operators[i-1] == '|') answer  = concat(answer, operands[i]);
    }
    return answer;
}

u8 eval_equation_combos(u64 expected, u64 *operands, u64 num_operands, u8 should_concat)
{
    u64 num_operators = num_operands - 1;
    u64 n_max         = 1 << num_operators;
    u64 c_max         = (should_concat) ? n_max : 1;
    u8 *operators     = (u8*)alloc(num_operators*sizeof(u8));
    u8 got_expected   = 0;
    for(u64 c = 0; c < c_max; c += 1)
    {
        for(u64 n = 0; n < n_max; n += 1)
        {
            gen_operator_list(operators, num_operators, n, c);
            u64 answer = eval_equation(operands, operators, num_operands);
            got_expected = (answer == expected);
            if(got_expected) break;
        }
        if(got_expected) break;
    }
    dealloc(operators);
    return got_expected;
}

u64 do_equations(equation_list *equations, u8 should_concat)
{
    u64 test_sum      = 0;
    u64 operand_index = 0;
    for(u64 i = 0; i < equations->num_equations; i += 1)
    {
        u64  len      = equations->lens[i];
        u64 *equation = &equations->operands[operand_index]; 

        if(!equations->is_true[i] && eval_equation_combos(equation[0], &equation[1], len-1, should_concat))
        {
            equations->is_true[i] = 1;
            test_sum += equation[0];
        }

        operand_index += len;
    }

    return test_sum;
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = (char*)read_file_contents("aoc_7.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
#endif

    tokeniser t = init_tokeniser(in_file_contents, in_file_size);

    u64 num_equations = 0;
    u64 num_operands = 0;

    for(token tok = read_token(&t); tok.type != TOKEN_END; tok = read_token(&t))
    {
        if(tok.type == TOKEN_INTEGER) num_operands  += 1;
        if(tok.type == TOKEN_NEWLINE) num_equations += 1;
    }

    printf("Num equations = %u\n", num_equations);
    equation_list equations = {0};
    equations.is_true       = (u8*)alloc(num_equations * sizeof(u8));
    equations.lens          = (u64*)alloc(num_equations * sizeof(u64));
    equations.operands      = (u64*)alloc(num_operands  * sizeof(u64));
    equations.num_equations = num_equations;

    t = init_tokeniser(in_file_contents, in_file_size);

    u64 operand_index  = 0;
    for(u64 i = 0; i < num_equations; i += 1)
    {
        token tok = read_token(&t); //INTEGER
        equations.operands[operand_index] = tok.int_val;
        operand_index += 1;
        tok = read_token(&t); //COLON
        
        u64 len = 1;
        for(tok = read_token(&t); tok.type != TOKEN_NEWLINE; tok = read_token(&t), len += 1)
        {
            tok = read_token(&t); //Operand
            equations.operands[operand_index] = tok.int_val;
            operand_index += 1;
        }
        equations.lens[i] = len;
    }

    u64 test_sum = do_equations(&equations, 0);
    printf("Part 1: Test sum = %lu\n", test_sum);

    u64 new_sum = do_equations(&equations, 1);
    printf("Part 2: Test sum = %lu\n", test_sum + new_sum);
    return 0;
}
