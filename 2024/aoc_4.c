#include "util.h"
#include <string.h>

u8 up_mask = 0b00000001;
u8 ur_mask = 0b00000010;
u8 ri_mask = 0b00000100;
u8 dr_mask = 0b00001000;
u8 do_mask = 0b00010000;
u8 dl_mask = 0b00100000;
u8 le_mask = 0b01000000;
u8 ul_mask = 0b10000000;

//Very bad way of getting a character from the wordsearch but quick to write
char get_ws_char(char *ws, u32 x, u32 y, u32 w)
{
    return ws[y*w + x];
}

u32 count_x_mas(u8 *ds, u32 sx, u32 sy, u32 ws_w, u32 ws_h)
{
    u32 num_x_mas = 0;
    u32 s_index = sy*ws_w + sx;
    u8 s = ds[s_index];

    //If diagonal up right
    if(s & ur_mask)
    {
        u32 a_index = (sy-2)*ws_w + sx;
        u32 r_index = sy*ws_w + (sx+2);
        u8 a = ds[a_index]; //Above
        u8 r = ds[r_index]; //Right

        if(a & dr_mask)
        {//X-mas
            num_x_mas += 1;
            a ^= dr_mask;
            ds[a_index] = a;
        }
        else if(r & ul_mask)
        {//X-mas
            num_x_mas += 1;
            r ^= ul_mask;
            ds[r_index] = r;
        }
        s ^= ur_mask;
    }
    //If diagonal down right
    if(s & dr_mask)
    {
        u32 b_index = (sy+2)*ws_w + sx;
        u32 r_index = sy*ws_w + (sx+2);
        u8 b = ds[b_index]; //Below
        u8 r = ds[r_index]; //Right

        if(b & ur_mask)
        {
            num_x_mas += 1;
            b ^= ur_mask;
            ds[b_index] = b;
        }
        else if(r & dl_mask)
        {
            num_x_mas += 1;
            r ^= dl_mask;
            ds[r_index] = r;
        }
        s ^= dr_mask;
    }
    //If diagonal down left
    if(s & dl_mask)
    {
        u32 b_index = (sy+2)*ws_w + sx;
        u32 l_index = sy*ws_w + (sx-2);
        u8 b = ds[b_index]; //Below
        u8 l = ds[l_index]; //Left

        if(b & ul_mask)
        {
            num_x_mas += 1;
            b ^= ul_mask;
            ds[b_index] = b;
        }
        else if(l & dr_mask)
        {
            num_x_mas += 1;
            l ^= dr_mask;
            ds[l_index] = l;
        }
        s ^= dl_mask;
    }
    //If diagonal up left
    if(s & ul_mask)
    {
        u32 a_index = (sy-2)*ws_w + sx;
        u32 l_index = sy*ws_w + (sx-2);
        u8 a = ds[a_index]; //Above
        u8 l = ds[l_index]; //Left

        if(a & dl_mask)
        {
            num_x_mas += 1;
            a ^= dl_mask;
            ds[a_index] = a;
        }
        else if(l & ur_mask)
        {
            num_x_mas += 1;
            l ^= ur_mask;
            ds[l_index] = l;
        }
        s ^= ul_mask;
    }
    ds[s_index] = s;

    return num_x_mas;
}

void count_str(char *ws, u8 *ds, u32 sx, u32 sy, u32 ws_w, u32 ws_h, const char *str, u32 str_len)
{
    char str_buf[4] = {0};

    //Up
    if(sy >= (str_len-1))
    {
        for(u32 i = 0; i < str_len; i += 1)
        {
            str_buf[i] = get_ws_char(ws, sx, sy-i, ws_w);
        }

        if(str_eq(str_buf, str_len, str, str_len)) ds[sy*ws_w + sx] |= up_mask;
    }

    //Up right
    if(sy >= (str_len-1) && sx < ws_w - (str_len-1))
    {
        for(u32 i = 0; i < str_len; i += 1)
        {
            str_buf[i] = get_ws_char(ws, sx+i, sy-i, ws_w);
        }

        if(str_eq(str_buf, str_len, str, str_len)) ds[sy*ws_w + sx] |= ur_mask;
    }

    //Right
    if(sx < ws_w - (str_len-1))
    { 
        for(u32 i = 0; i < str_len; i += 1)
        {
            str_buf[i] = get_ws_char(ws, sx+i, sy, ws_w);
        }

        if(str_eq(str_buf, str_len, str, str_len)) ds[sy*ws_w + sx] |= ri_mask;
    }

    //Down right
    if(sy < ws_h - (str_len-1) && sx < ws_w - (str_len-1))
    {
        for(u32 i = 0; i < str_len; i += 1)
        {
            str_buf[i] = get_ws_char(ws, sx+i, sy+i, ws_w);
        }

        if(str_eq(str_buf, str_len, str, str_len)) ds[sy*ws_w + sx] |= dr_mask;
    }

    //Down
    if(sy < ws_h - (str_len-1))
    {
        for(u32 i = 0; i < str_len; i += 1)
        {
            str_buf[i] = get_ws_char(ws, sx, sy+i, ws_w);
        }

        if(str_eq(str_buf, str_len, str, str_len)) ds[sy*ws_w + sx] |= do_mask;
    }

    //Down left
    if(sx >= (str_len-1) && sy < ws_h - (str_len-1))
    {
        for(u32 i = 0; i < str_len; i += 1)
        {
            str_buf[i] = get_ws_char(ws, sx-i, sy+i, ws_w);
        }

        if(str_eq(str_buf, str_len, str, str_len)) ds[sy*ws_w + sx] |= dl_mask;
    }

    //Left
    if(sx >= (str_len-1))
    {
        for(u32 i = 0; i < str_len; i += 1)
        {
            str_buf[i] = get_ws_char(ws, sx-i, sy, ws_w);
        }

        if(str_eq(str_buf, str_len, str, str_len)) ds[sy*ws_w + sx] |= le_mask;
    }

    //Up left
    if(sx >= (str_len-1) && sy >= (str_len-1))
    {
        for(u32 i = 0; i < str_len; i += 1)
        {
            str_buf[i] = get_ws_char(ws, sx-i, sy-i, ws_w);
        }

        if(str_eq(str_buf, str_len, str, str_len)) ds[sy*ws_w + sx] |= ul_mask;
    }
}

int main()
{
    u32 in_file_size;
    char *in_file_contents = (char*)read_file_contents("aoc_4.txt", &in_file_size);

    u32 w = 0;
    u32 h = 0;
    for(; in_file_contents[w] != '\n'; w += 1);
    for(u32 i = 0; i < in_file_size; i += 1)
    {
        if(in_file_contents[i] == '\n') h += 1;
    }

    char *ws = (char*)alloc(w*h);
    u8   *ds = (u8*)alloc(w*h);

    for(u32 y = 0; y < h; y += 1)
    {
        for(u32 x = 0; x < w; x += 1)
        {
            ws[y*w + x] = in_file_contents[y*(w+1) + x];
        }
    }

    for(u32 y = 0; y < h; y += 1)
    {
        for(u32 x = 0; x < w; x += 1)
        {
            count_str(ws, ds, x, y, w, h, "XMAS", 4);
        }
    }

    u32 num_xmas = 0;
    for(u32 i = 0; i < w*h; i += 1)
    {
        num_xmas += __builtin_popcount(ds[i]);
    }

    printf("Part 1: Num xmas = %u\n", num_xmas);

    for(u32 i = 0; i < w*h; i += 1) ds[i] = 0;

    for(u32 y = 0; y < h; y += 1)
    {
        for(u32 x = 0; x < w; x += 1)
        {
            count_str(ws, ds, x, y, w, h, "MAS", 3);
        }
    }

    u32 num_x_mas = 0;

    for(u32 y = 0; y < h; y += 1)
    {
        for(u32 x = 0; x < w; x += 1)
        {
            u32 n = count_x_mas(ds, x, y, w, h);
            num_x_mas += n;
        }
    }

    printf("Part 2: Num x_mas = %u\n", num_x_mas);

    return 0;
}
