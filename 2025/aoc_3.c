#include "util.h"

// Look for the first character larger than c in d from &d[0] to &d[d_len-1]
u32 find_supplicant(char *d, u32 d_len, char c)
{
    for(u32 i = 0; i < d_len; i += 1) if(c > d[i]) return i;
    return -1;
}

// Replace &dst[supplicant] to &dst[dst_len-1] with &src[0] to &src[dst_len-supplicant-1]
void supplant(char *dst, u32 dst_len, u32 supplicant, char *src)
{
    for(u32 i = supplicant, j = 0; i < dst_len; i += 1, j += 1) dst[i] = src[j];
}

void find_largest(char *str, u32 len, char *ret, u32 ret_len)
{
    supplant(ret, ret_len, 0, str);
    u32 p = ret_len;
    for(u32 i = 1; i < (len - ret_len); i += 1)
    {
        char *c = &str[i];
        u32 d_len = (p <= i) ? ret_len : (ret_len - (p - i));
        u32 n = find_supplicant(ret, d_len, *c);
        if(n != -1)
        {
            supplant(ret, ret_len, n, c);
            p = i + (ret_len - n);
        }
    }
    for(u32 i = (len - ret_len); i < len; i += 1, ret_len -= 1, ret += 1)
    {
        char *c = &str[i];
        u32 d_len = (p <= i) ? ret_len : (ret_len - (p - i));
        u32 n = find_supplicant(ret, d_len, *c);
        if(n != -1)
        {
            supplant(ret, ret_len, n, c);
            p = i + (ret_len - n);
        }
    }
}

const char example[] =
{
    "987654321111111\n"
    "811111111111119\n"
    "234234234234278\n"
    "818181911112111\n"
};

u64 chars_to_u64(char *c, u32 l)
{
    u64 u = 0;
    u64 b = 1;
    for(u32 i = l; i > 0; i -= 1, b *= 10)
    {
        u64 n = (u64)(c[i-1] - '0');
        u += b * n;
    }
    return u;
}

void print_bank(char *str, u64 l)
{
    printf("%.*s", l, str);
}

u64 compute_voltage(char *in, u32 bank_len, u32 num_banks, char *volt, u32 volt_len)
{
    u64 sum = 0;
    for(u32 b = 0, i = 0; b < num_banks; b += 1, i += bank_len+1)
    {
        char *str = &in[i];
        find_largest(str, bank_len, volt, volt_len);
        u64 s = chars_to_u64(volt, volt_len);
        sum += s;
    }
    return sum;
}

int main()
{
#if 0
    const char *in = example;
    u32 in_len = strlen(example);
#else
    u64 in_len;
    char *in = read_file_contents("aoc_3.txt", &in_len);
#endif
    u32 bank_len;
    for(bank_len = 0; in[bank_len] != '\n'; bank_len += 1);
    u32 num_banks = 0;
    for(char *c = in; *c != 0; c += 1) if(*c == '\n') num_banks += 1;
    printf("Bank len = %lu Num banks = %lu\n", bank_len, num_banks);
    char volt[12];
    u32 volt_len = 2;
    printf("Sum = %llu\n", compute_voltage(in, bank_len, num_banks, volt, volt_len));
    volt_len = 12;
    printf("Sum = %llu\n", compute_voltage(in, bank_len, num_banks, volt, volt_len));
    return 0;
}
