#include <stdio.h>
#include <stdint.h>
#include <windows.h>

typedef uint64_t u64;

const char *find_next_char(const char *str, char c)
{
    for(; *str != c; ++str);
    return str;
}

const char *find_next_non_space(const char *str)
{
    for(; *str == ' '; ++str);
    return str;
}

u64 is_number(const char *str)
{
    return '0' <= *str && *str <= '9';
}

u64 str_to_num(const char *str)
{
    u64 n = 0;
    u64 base = 1;

    const char *num_end = str;
    for(; is_number(num_end); ++num_end);
    --num_end;

    for(u64 digit; num_end >= str; --num_end)
    {
        digit =  *num_end - '0';
        n     += base * digit;
        base  *= 10;
    }

    return n;
}

typedef struct hand hand;

typedef enum
{
    HAND_NONE,
    HAND_HIGH,
    HAND_ONE,
    HAND_TWO,
    HAND_THREE,
    HAND_FULL,
    HAND_FOUR,
    HAND_FIVE,
    HAND_SENT //Sentinel
} hand_type;

struct hand
{
    char cards[5];
    u64  bid;
    u64  type;

    hand *prev;
    hand *next;
};

//Adjacent hands
void swap_hands(hand *h0, hand *h1)
{
    h0->prev->next = h1;
    h1->next->prev = h0;

    hand *tmp;
    tmp = h0->prev;
    h0->prev = h1;
    h0->next = h1->next;
    h1->next = h0;
    h1->prev = tmp;
}

hand *append_hand(hand *l, hand *h)
{
    h->next = l->next;
    h->prev = l;
    l->next = h;

    return h;
}

hand *get_hand(hand *h, u64 n)
{
    for(u64 i = 0; i < n; ++i, h = h->next);
    return h;
}

u64 card_value(char c)
{
    if('2' <= c && c <= '9') return c - '2' + 1;
    switch(c)
    {
        case 'T': return 9;
        case 'J': return 0;
        case 'Q': return 10;
        case 'K': return 11;
        case 'A': return 12;
    }
}

u64 higher_card_first(hand *h0, hand *h1)
{
    for(u64 i = 0; i < 5; ++i)
    {
        if(card_value(h0->cards[i]) > card_value(h1->cards[i])) return 1;
        if(card_value(h1->cards[i]) > card_value(h0->cards[i])) return 0;
    }
    return 0;
}

void print_hand(hand *h)
{
    printf("%.5s | type: ", h->cards);

    switch(h->type)
    {
        case HAND_HIGH:  printf("HIGH");  break;
        case HAND_ONE:   printf("ONE");   break;
        case HAND_TWO:   printf("TWO");   break;
        case HAND_THREE: printf("THREE"); break;
        case HAND_FULL:  printf("FULL");  break;
        case HAND_FOUR:  printf("FOUR");  break;
        case HAND_FIVE:  printf("FIVE");  break;
        default:         printf("???");
    }

    printf(" ");
}

void find_hand_type(hand *h)
{
    u64  num_unique_cards = 0;
    char cards[5]         = {};
    u64  card_counts[5]   = {};

    for(u64 i = 0; i < 5; ++i)
    {
        char c = h->cards[i];
        u64 is_unique = 1;
        for(u64 j = 0; j < num_unique_cards; ++j)
        {
            if(cards[j] == c)
            {
                card_counts[j] += 1;
                is_unique = 0;
                break;
            }
        }
        if(is_unique)
        {
            cards[num_unique_cards] = c;
            card_counts[num_unique_cards] = 1;
            num_unique_cards += 1;
        }
    }

    u64 len = num_unique_cards;
    for(u64 swapped = 1; swapped == 1;)
    {
        swapped = 0;
        for(u64 i = 0; i < len - 1; ++i)
        {
            if(card_counts[i] < card_counts[i+1])
            {
                u64 tmp = card_counts[i];
                char tmp_c = cards[i];
                card_counts[i] = card_counts[i+1];
                cards[i] = cards[i+1];
                card_counts[i+1] = tmp;
                cards[i+1] = tmp_c;
                swapped = 1;
            }
        }
        len -= 1;
    }

    if(num_unique_cards == 1) h->type = HAND_FIVE;
    if(num_unique_cards == 5) h->type = HAND_HIGH;
    if(card_counts[0] == 4 && card_counts[1] == 1) h->type = HAND_FOUR;
    if(card_counts[0] == 3 && card_counts[1] == 2) h->type = HAND_FULL;
    else if(card_counts[0] == 3) h->type = HAND_THREE;
    if(card_counts[0] == 2 && card_counts[1] == 2) h->type = HAND_TWO;
    else if(card_counts[0] == 2) h->type = HAND_ONE;

    u64 ji = 0;
    for(; ji < num_unique_cards && cards[ji] != 'J'; ++ji);

    u64 to_swap = card_counts[ji];
    if(ji < num_unique_cards)
    {
        for(u64 i = 0; i <= ji; ++i)
        {
            u64 tmp = card_counts[i];
            card_counts[i] = to_swap;
            to_swap = tmp;
        }

        u64 num_js = card_counts[0];
        u64 num_other = card_counts[1];
        if(num_js == 5 || num_js == 4     || 
          (num_js == 3 && num_other == 2) ||
          (num_js == 2 && num_other == 3) ||
          (num_js == 1 && num_other == 4))
        { 
            h->type = HAND_FIVE;
        }
        if((num_js == 3 && num_other == 1) ||
           (num_js == 2 && num_other == 2) ||
           (num_js == 1 && num_other == 3))
        {
            h->type = HAND_FOUR;
        }
        if(num_js == 1 && num_other == 2 && card_counts[2] == 2)
        {
            h->type = HAND_FULL;
        }
        else if((num_js == 2 && num_other == 1) ||
           (num_js == 1 && num_other == 2))
        {
            h->type = HAND_THREE;
        }
        if(num_js == 1 && num_other == 1)
        {
            h->type = HAND_ONE;
        }
    }
}

const char in_arg[] =
#if 0
"32T3K 765\n"
"T55J5 684\n"
"KK677 28\n"
"KTJJT 220\n"
"QQQJA 483\n";
#else
#include "aoc_2023_7.txt"
#endif

int main()
{
    
    u64 in_len = sizeof(in_arg) - 1;
    const char *in_end = in_arg + in_len;
    
    u64 num_hands = 0;
    for(const char *c = in_arg; c < in_end; ++c, ++num_hands)
    {
        c = find_next_char(c, '\n');
        c = find_next_non_space(c);
    }

    printf("%u\n", num_hands);

    hand *hands = VirtualAlloc(NULL, (num_hands+1) * sizeof(hand), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    hand *sentinel = &hands[0];
    sentinel->bid = 0xdeadbeef;
    sentinel->type = HAND_SENT;
    sentinel->prev = &hands[0];
    sentinel->next = &hands[0];

    const char *c = in_arg;
    hand *end = sentinel;
    for(u64 i = 0; i < num_hands; ++i)
    {
        hand *h = &hands[i+1];
        for(u64 j = 0; j < 5; ++j, ++c)
        {
            h->cards[j] = *c;
        }

        c = find_next_non_space(c);
        h->bid = str_to_num(c);
        c = find_next_char(c, '\n');
        c += 1;

        end = append_hand(end, h);
    }

    for(hand *h = sentinel->next; h->type != HAND_SENT; h = h->next)
    {
        find_hand_type(h);
    }

    printf("===================================\n");

    //Sort hands by type
    u64 len = num_hands;
    for(u64 swapped = 1; swapped == 1;)
    {
        swapped = 0;
        hand *h = sentinel->next;
        for(u64 i = 0; i < len - 1; ++i)
        {
            if(h->type > h->next->type)
            {
                //swap
                swap_hands(h, h->next);
                h = h->prev;
                swapped = 1;
            } 
            h = h->next;
        }
        len -= 1;
    }

    //Sort hands by highest card
    for(u64 start = 0; start < num_hands;)
    {
        hand *h = get_hand(sentinel->next, start);

        u64 type_num = 1;
        for(hand *end_h = get_hand(sentinel->next, start); end_h->type == end_h->next->type; end_h = end_h->next, ++type_num);

        u64 len = type_num;
        for(u64 swapped = 1; swapped == 1;)
        {
            swapped = 0;
            hand *h0 = get_hand(sentinel->next, start);
            for(u64 i = 1; i < len; ++i)
            {
                if(higher_card_first(h0, h0->next))
                {
                    swap_hands(h0, h0->next);
                    h0 = h0->prev;
                    swapped = 1;
                }
                h0 = h0->next;
            }
            len -= 1;
        }

        start += type_num;
    }

    for(hand *h = sentinel->next; h->type != HAND_SENT; h = h->next)
    {
        print_hand(h);
        printf("\n");
    }

    u64 winnings = 0;
    u64 rank = 1;
    for(hand *h = sentinel->next; h->type != HAND_SENT; h = h->next)
    {
        winnings += rank * h->bid;
        rank += 1;
    }
    printf("Winnings: %u\n", winnings);
    return 0;
}
