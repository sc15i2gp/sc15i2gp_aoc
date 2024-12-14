#include "util.h"

//NOTE: BIG NOTE
//Today was a total clusterfuck. I saw this problem and thought
//"Hey, this is an optimisation problem!" so I tried implementing
//simplex. I'm glad I relearned the basic method but what a blunder!

char test[] =
"Button A: X+94, Y+34\n"
"Button B: X+22, Y+67\n"
"Prize: X=8400, Y=5400\n"
"\n"
"Button A: X+26, Y+66\n"
"Button B: X+67, Y+21\n"
"Prize: X=12748, Y=12176\n"
"\n"
"Button A: X+17, Y+86\n"
"Button B: X+84, Y+37\n"
"Prize: X=7870, Y=6450\n"
"\n"
"Button A: X+69, Y+23\n"
"Button B: X+27, Y+71\n"
"Prize: X=18641, Y=10279\n";

typedef struct
{
    union
    {
        struct
        {
            f64 c_a;
            f64 c_b;
            f64 rhs;
        };
        f64 vars[3];
    };
} cnstr;

typedef struct
{
    union
    {
        struct
        {
            cnstr x_cnstr;
            cnstr y_cnstr;
            cnstr z_cnstr;
        };
        cnstr cnstrs[3];
    };
    u64 orig_vars[6];
} tableau;

typedef struct
{
    u64      num_problems;
    tableau *problems;
    tableau *solutions;
} tableau_list;

void print_cnstr(cnstr *c)
{
    printf("%f * A + %f * B = %f", c->c_a, c->c_b, c->rhs);
}

void print_tableau(tableau *t)
{
    printf("X   | %f  | %f  | %f\n", t->x_cnstr.c_a, t->x_cnstr.c_b, t->x_cnstr.rhs);
    printf("Y   | %f  | %f  | %f\n", t->y_cnstr.c_a, t->y_cnstr.c_b, t->y_cnstr.rhs);
    printf("Z   | %f  | %f  | %f\n", t->z_cnstr.c_a, t->z_cnstr.c_b, t->z_cnstr.rhs);
    u64 ax = t->orig_vars[0]; u64 bx = t->orig_vars[2];
    u64 ay = t->orig_vars[1]; u64 by = t->orig_vars[3];
    u64 x = t->orig_vars[4]; u64 y = t->orig_vars[5];
    printf("Prob: A(X+%lu, Y+%lu), B(X+%lu, Y+%lu), X=%lu, Y=%lu\n", ax, ay, bx, by, x, y);
}

u8 cnstr_holds(cnstr *c, f64 a, f64 b)
{
    f64 r = c->c_a * a + c->c_b * b;
    return r == c->rhs;
}

u8 feq_approx(f64 f, f64 g)
{
    f64 eps = 0.0000001;
    if(f > g) return ((f - g) <= eps);
    else      return ((g - f) <= eps);
}

u8 is_integer(f64 f)
{
    return (feq_approx(f, floor(f)) || feq_approx(f, ceil(f)));
}

u8 is_factor(f64 f, f64 g)
{
    f64 d = g/f;
    return is_integer(d);
}

u64 to_int(f64 f)
{
    f64 ff = floor(f);
    f64 fc = ceil(f);
    if(f == ff) return ff;
    if(f == fc) return fc;

    f64 df = fabs(ff - f);
    f64 dc = fabs(fc - f);

    if(df < dc) return ff;
    else        return fc;
}

void get_solutions(tableau *solution, f64 *a, f64 *b)
{
    for(u64 i = 0; i < 3; i += 1)
    {
        if(solution->cnstrs[i].vars[0] == 1.0) *a = solution->cnstrs[i].vars[2];
        if(solution->cnstrs[i].vars[1] == 1.0) *b = solution->cnstrs[i].vars[2];
    }
}

//To be solved:
// - Solutions must be integers
// - A and B must be less than 100
// - Z constraint's a and b must be 0
// - X and Y constraint's a and b must have a single 1.0 between them
u8 is_problem_solved(tableau *problem, tableau *solution)
{
    f64 a, b;
    get_solutions(solution, &a, &b);

    f64 za = solution->z_cnstr.c_a;
    f64 zb = solution->z_cnstr.c_b;
    f64 xa = solution->x_cnstr.c_a;
    f64 xb = solution->x_cnstr.c_b;
    f64 ya = solution->y_cnstr.c_a;
    f64 yb = solution->y_cnstr.c_b;

    u8 x_sat = (xa == 1.0 && xb == 0.0) || (xa == 0.0 && xb == 1.0);
    u8 y_sat = (ya == 1.0 && yb == 0.0) || (ya == 0.0 && yb == 1.0);
    u8 z_sat = (za == 0.0 && zb == 0.0);

    return x_sat && y_sat && z_sat &&
           a <= 100.0 && b <= 100.0 &&
           is_integer(a) && is_integer(b);
}

tableau_list parse_problems(char *in, u64 in_size)
{
    tokeniser t = init_tokeniser(in, in_size);

    u64 num_tableau = 0;
    for(token tok = read_token(&t); tok.type != TOKEN_END; tok = read_token(&t))
    {
        if(tok.type == TOKEN_WORD && str_eq(tok.loc, tok.len, "Prize", 5))
        {
            num_tableau += 1;
        }
    }
    printf("Num tableau = %lu\n", num_tableau);

    t = init_tokeniser(in, in_size);
    cnstr min_cnstr = {-1.0, -1.0, 0.0};
    tableau *problems  = (tableau*)alloc(num_tableau * sizeof(tableau));
    tableau *solutions = (tableau*)alloc(num_tableau * sizeof(tableau));

    for(u64 i = 0; i < num_tableau; i += 1)
    {
        tableau *dst = &problems[i];
        tableau *sol = &solutions[i];

        token tok;
        for(u64 j = 0; j < 8; j += 1) tok = read_token(&t); //Get to first constraint variable
        if(tok.type != TOKEN_INTEGER) break;
        dst->x_cnstr.c_a  = (f64)tok.int_val;
        dst->orig_vars[0] = tok.int_val;
        for(u64 j = 0; j < 5; j += 1) tok = read_token(&t); //Get to second constraint variable
        if(tok.type != TOKEN_INTEGER) break;
        dst->y_cnstr.c_a  = (f64)tok.int_val;
        dst->orig_vars[1] = tok.int_val;
        for(u64 j = 0; j < 9; j += 1) tok = read_token(&t); //Get to third constraint variable
        if(tok.type != TOKEN_INTEGER) break;
        dst->x_cnstr.c_b  = (f64)tok.int_val;
        dst->orig_vars[2] = tok.int_val;
        for(u64 j = 0; j < 5; j += 1) tok = read_token(&t); //Get to fourth constraint variable
        if(tok.type != TOKEN_INTEGER) break;
        dst->y_cnstr.c_b  = (f64)tok.int_val;
        dst->orig_vars[3] = tok.int_val;
        for(u64 j = 0; j < 7; j += 1) tok = read_token(&t); //Get to first rhs
        if(tok.type != TOKEN_INTEGER) break;
        dst->x_cnstr.rhs  = (f64)tok.int_val;
        dst->orig_vars[4] = tok.int_val;
        for(u64 j = 0; j < 5; j += 1) tok = read_token(&t); //Get to second rhs
        if(tok.type != TOKEN_INTEGER) break;
        dst->y_cnstr.rhs  = (f64)tok.int_val;
        dst->orig_vars[5] = tok.int_val;

        //Copy in min constraint
        dst->z_cnstr = min_cnstr;

        for(u64 j = 0; j < 3; j += 1) sol->cnstrs[j] = dst->cnstrs[j];
        for(u64 j = 0; j < 6; j += 1) sol->orig_vars[j] = dst->orig_vars[j];

        tok = read_token(&t);
        tok = read_token(&t);
    }

    tableau_list problem_list = {num_tableau, problems, solutions};

    return problem_list;
}

void solve_problem(tableau *t)
{
    for(u64 n = 0; n < 2; n += 1)
    {
        //Find pivot row and column
        u64 pc = 0;
        for(u64 i = 0; i < 3; i += 1)
        {
            f64 f = t->z_cnstr.vars[i];
            if(f < 0.0 && f < t->z_cnstr.vars[pc]) pc = i;
        }
        f64 fc0 = t->cnstrs[0].vars[pc];
        f64 fr0 = t->cnstrs[0].rhs;
        f64 fc1 = t->cnstrs[1].vars[pc];
        f64 fr1 = t->cnstrs[1].rhs;
        
        f64 f0 = fr0/fc0;
        f64 f1 = fr1/fc1;

        u64 pr = (f0 < f1) ? 0  : 1;
        f64 f  = t->cnstrs[pr].vars[pc];
        
        //Divide pivot row by pivot value
        for(u64 i = 0; i < 3; i += 1) t->cnstrs[pr].vars[i] /= f;

        //Normalise pivot column
        for(u64 i = 0; i < 3; i += 1)
        {
            if(i != pr)
            {
                f64 d = t->cnstrs[i].vars[pc];
                for(u64 j = 0; j < 3; j += 1)
                {
                    t->cnstrs[i].vars[j] -= (d * t->cnstrs[pr].vars[j]);
                }
            }
        }
    }
}

void day_11_simplex(char *in_file_contents, u64 in_file_size)
{
    cnstr c = {94.0, 22.0, 8400.0};
    tableau t;
    t.x_cnstr = c;
    t.y_cnstr = c;
    t.z_cnstr = c;
    print_tableau(&t);
    print_cnstr(&c); printf("\n");
    printf("Constraint holds: %u\n", cnstr_holds(&c, 80.0, 40.0));
    tableau_list problems = parse_problems(in_file_contents, in_file_size);

    for(u64 i = 0; i < problems.num_problems; i += 1)
    {
        solve_problem(&problems.solutions[i]);
    }

    u64 num_tokens = 0;
    for(u64 i = 0; i < problems.num_problems; i += 1)
    {
        printf("%lu: ", i);
        if(is_problem_solved(&problems.problems[i], &problems.solutions[i]))
        {
            f64 a,  b;
            u64 ua, ub;
            get_solutions(&problems.solutions[i], &a, &b);
            ua = to_int(a);
            ub = to_int(b);

            num_tokens += ((3 * ua) + ub);
            printf("Solutions: %f %f Num tokens = %lu\n", a, b, num_tokens);

            tableau *t = &problems.problems[i];
            u64 ax = t->orig_vars[0]; u64 ay = t->orig_vars[1];
            u64 bx = t->orig_vars[2]; u64 by = t->orig_vars[3];
            u64 sx = t->orig_vars[4]; u64 sy = t->orig_vars[5];
            u64 ex = ax * ua + bx * ub;
            u64 ey = ay * ua + by * ub;
            printf("X: %lu * %lu + %lu * %lu = %lu (%lu)\n", ax, ua, bx, ub, ex, sx);
            printf("Y: %lu * %lu + %lu * %lu = %lu (%lu)\n", ay, ua, by, ub, ey, sy);
            if(ex != sx || ey != sy) printf("DISCREPANCY!!!\n");
        }
        else printf("\n");
        print_tableau(&problems.solutions[i]); printf("\n");
    }
    printf("Part 1: Num tokens = %lu\n", num_tokens);
    printf("Solved? %lu\n", is_problem_solved(&problems.problems[2], &problems.solutions[2]));
}

typedef struct
{
    u64 ax;
    u64 ay;
    u64 bx;
    u64 by;
    u64 px;
    u64 py;
} machine;

u8 is_uint_factor(s64 f, s64 n)
{
    return (n % f == 0);
}

void print_machine(machine *m)
{
    printf("X: Na * %lu + Nb * %lu = %lu\n", m->ax, m->bx, m->px);
    printf("Y: Na * %lu + Nb * %lu = %lu\n", m->ay, m->by, m->py);
}

machine *parse_machines(char *in, u64 in_size, u64 *num_machines_ret)
{
    tokeniser t = init_tokeniser(in, in_size);
    
    u64 num_machines = 0;
    for(token tok = read_token(&t); tok.type != TOKEN_END; tok = read_token(&t))
    {
        if(tok.type == TOKEN_WORD && str_eq(tok.loc, tok.len, "Prize", 5))
        {
            num_machines += 1;
        }
    }

    t = init_tokeniser(in, in_size);

    machine *machines = (machine*)alloc(num_machines * sizeof(machine));

    for(u64 i = 0; i < num_machines; i += 1)
    {
        machine *dst = &machines[i];

        token tok;
        for(u64 j = 0; j < 8; j += 1) tok = read_token(&t);
        dst->ax = tok.int_val;
        for(u64 j = 0; j < 5; j += 1) tok = read_token(&t);
        dst->ay = tok.int_val;
        for(u64 j = 0; j < 9; j += 1) tok = read_token(&t);
        dst->bx = tok.int_val;
        for(u64 j = 0; j < 5; j += 1) tok = read_token(&t);
        dst->by = tok.int_val;
        for(u64 j = 0; j < 7; j += 1) tok = read_token(&t);
        dst->px = tok.int_val;
        for(u64 j = 0; j < 5; j += 1) tok = read_token(&t);
        dst->py = tok.int_val;

        tok = read_token(&t);
        tok = read_token(&t);
    }

    *num_machines_ret = num_machines;
    return machines;
}

u8 find_button_presses(machine *m, s64 *ra, s64 *rb)
{
    u8 valid_solution = 1;
    s64 d = (m->ax * m->by) - (m->ay * m->bx);
    s64 a = (m->by * m->px) - (m->bx * m->py);
    s64 b = (m->ax * m->py) - (m->ay * m->px);

    valid_solution = (is_uint_factor(d, a) && is_uint_factor(d, b));
    a /= d;
    b /= d;

    *ra = a;
    *rb = b;

    return valid_solution;
}

int main()
{
#if 1
    u64   in_file_size;
    char *in_file_contents = read_file_contents("aoc_13.txt", &in_file_size);
#else
    char *in_file_contents = test;
    u64   in_file_size     = str_len(test);
#endif
    //day_11_simplex(in_file_contents, in_file_size);
    u64 num_machines;
    machine *machines = parse_machines(in_file_contents, in_file_size, &num_machines);

    s64 num_tokens = 0;
    for(u64 i = 0; i < num_machines; i += 1)
    {
        s64 a, b;
        u8 valid = find_button_presses(&machines[i], &a, &b);

        if(valid) num_tokens += (3*a) + b;
    }
    printf("Part 1: Num tokens = %ld\n", num_tokens);

    for(u64 i = 0; i < num_machines; i += 1)
    {
        machines[i].px += 10000000000000;
        machines[i].py += 10000000000000;
    }
    num_tokens = 0;
    for(u64 i = 0; i < num_machines; i += 1)
    {
        s64 a, b;
        u8 valid = find_button_presses(&machines[i], &a, &b);

        if(valid) num_tokens += (3*a) + b;
    }
    printf("Part 2: Num tokens = %ld\n", num_tokens);
    return 0;
}
