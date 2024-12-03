#include "util.h"

u8 is_report_safe(u32 *levels, u32 len)
{
    u8 increasing = (levels[0] < levels[1]);
    for(u32 i = 0; i < len - 1; i += 1)
    {
        if(increasing) {if(levels[i] >= levels[i+1]) return 0;}
        else           {if(levels[i] <= levels[i+1]) return 0;}
    }

    //At this point the report is definitely increasing or decreasing
    for(u32 i = 0; i < len - 1; i += 1)
    {
        u32 difference = (increasing) ? levels[i+1] - levels[i] : levels[i] - levels[i+1];
        if(difference == 0 || difference > 3) return 0;
    }

    return 1;
}

u8 is_report_safe_dampened(u32 *levels, u32 len)
{
    u32 *d_levels = (u32*)alloc((len - 1) * sizeof(u32));

    //n^2 bad but quick to write
    for(u32 i = 0; i < len; i += 1)
    {
        //Populate d_levels, ignoring levels[i]
        u32 d_level_index = 0;
        for(u32 j = 0; j < len; j += 1)
        {
            if(j == i) continue;

            d_levels[d_level_index] = levels[j];
            d_level_index += 1;
        }

        //Test if new report is safe
        if(is_report_safe(d_levels, len-1)) return 1;
    }

    return 0;
}

typedef struct
{
    u32 num_reports;
    u32 *lens;
    u32 *levels;
} reports_list;

reports_list parse_reports(u8 *reports_contents, u32 reports_contents_len)
{
    reports_list reports = {0};

    tokeniser t = init_tokeniser(reports_contents, reports_contents_len);
    
    u32 num_reports = 0;
    u32 num_levels  = 0;
    for(token tok = read_token(&t); tok.type != TOKEN_END; tok = read_token(&t))
    {
        if(tok.type == TOKEN_NEWLINE) num_reports += 1;
        if(tok.type == TOKEN_INTEGER) num_levels  += 1;
    }

    u32 *lens        = (u32*)alloc(num_reports * sizeof(u32));
    u32 *levels      = (u32*)alloc(num_levels  * sizeof(u32));
    u32  level_index = 0;
    t = init_tokeniser(reports_contents, reports_contents_len);
    for(u32 i = 0; i < num_reports; i += 1)
    {
        u32 len = 0;
        for(token tok = read_token(&t); tok.type != TOKEN_NEWLINE && tok.type != TOKEN_END; tok = read_token(&t))
        {
            if(tok.type == TOKEN_INTEGER)
            {
                levels[level_index] = tok.int_val;

                len         += 1;
                level_index += 1;
            }
        }
        lens[i] = len;
    }

    reports.num_reports = num_reports;
    reports.lens        = lens;
    reports.levels      = levels;
    return reports;
}

int main()
{
    u32 in_file_size;
    u8 *in_file_contents = read_file_contents("aoc_2.txt", &in_file_size);

    reports_list reports = parse_reports(in_file_contents, in_file_size);
    printf("Num reports = %u\n", reports.num_reports);

    u32 num_safe = 0;
    u32 levels_offset = 0;
    for(u32 i = 0; i < reports.num_reports; i += 1)
    {
        u32  report_len    = reports.lens[i];
        u32 *report_levels = reports.levels + levels_offset;

        u8 report_safe = is_report_safe(report_levels, report_len);
        if(report_safe) num_safe += 1;

        levels_offset += report_len;
    }
    
    printf("Part 1: Num safe reports = %u\n", num_safe);

    num_safe = 0;
    levels_offset = 0;
    for(u32 i = 0; i < reports.num_reports; i += 1)
    {
        u32  report_len    = reports.lens[i];
        u32 *report_levels = reports.levels + levels_offset;

        u8 report_safe = is_report_safe_dampened(report_levels, report_len);
        if(report_safe) num_safe += 1;

        levels_offset += report_len;
    }
    
    printf("Part 1: Num safe reports dampened = %u\n", num_safe);
    return 0;
}
