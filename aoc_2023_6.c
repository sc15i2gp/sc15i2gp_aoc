#include <stdio.h>
#include <stdint.h>

typedef uint64_t u64;

u64 ways_to_win(u64 time, u64 target)
{
    u64 speed;
    for(speed = 1; speed <= time/2; ++speed)
    {
        u64 dist = speed * (time - speed);
        if(dist > target) break;
    }

    u64 ways = time - (2*speed) + 1;
    return ways;
}

int main()
{
    u64 r = ways_to_win(40828492, 233101111101487);
    printf("%u\n", r);
    return 0;
}
