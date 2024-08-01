// https://www.youtube.com/watch?v=8sEe-4tig_A
#include <stdio.h>
#include "stc/coroutine.h"
#define i_type IVec,int
#include "stc/vec.h"

struct GenValue {
    IVec *v;
    IVec_iter it;
    int cco_state;
};

static long get_value(struct GenValue* g)
{
    cco_scope(g) {
        cco_foreach (g->it, IVec, *g->v)
            cco_yield_v(*g->it.ref);
    }
    return 1L<<31;
}

struct Generator {
    struct GenValue x, y;
    int cco_state;
    bool xact, yact;
    long value;
};

int interleaved(struct Generator* g)
{
    cco_scope(g) {
        do {
            g->value = get_value(&g->x);
            g->xact = cco_active(&g->x);
            if (g->xact)
                cco_yield;

            g->value = get_value(&g->y);
            g->yact = cco_active(&g->y);
            if (g->yact)
                cco_yield;
        } while (g->xact | g->yact);
    }
    return 0;
}

void Use(void)
{
    IVec a = c_init(IVec, {2, 4, 6, 8, 10, 11});
    IVec b = c_init(IVec, {3, 5, 7, 9});

    struct Generator g = {{&a}, {&b}};

    cco_run_coroutine(interleaved(&g)) {
        printf("%ld ", g.value);
    }
    puts("");
    c_drop(IVec, &a, &b);
}

int main(void)
{
    Use();
}
