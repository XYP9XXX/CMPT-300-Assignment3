#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct mem_control_block
{
    bool free;
    struct mem_control_block* prev;
    size_t size;
}MCB;

MCB* g_top = NULL;

void* my_malloc(size_t size) {
    MCB* mcb;
    for (mcb = g_top; mcb; mcb = mcb->prev) {
        if (mcb->free && mcb->size) {
            break;
        }
    }

    if (!mcb) {
        mcb = sbrk(sizeof(MCB) + size);
        if (mcb == (void*) -1) {
            perror("sbrk");
            return NULL;
        }
        mcb->prev = g_top;
        mcb->size = size;
        g_top = mcb;
    }
    mcb->free = false;
    return mcb + 1;
}

void my_free (void* ptr) {
    if (!ptr) {
        return;
    }
     MCB* mcb = (MCB*)ptr - 1;
    mcb->free = true;
    for (mcb = g_top; mcb->prev != NULL && mcb->free; mcb = mcb->prev) {
        if (mcb->free) {
            g_top = mcb->prev;
            brk(mcb);
        } else {
            g_top = mcb;
            brk((void*)mcb + mcb->size + sizeof(MCB));
        }
    }
}

int main(void) {
    int* pa[10];
    size_t size = sizeof(pa) / sizeof(pa[0]), i, j;
    for (i = 0; i < size; i++) {
        if (!(pa[i] = (int*)my_malloc((i + 1) * sizeof(int)))) {
            perror("my_malloc");
            return -1;
        }
        for (j = 0; j <= i; j++) {
            pa[i][j] = j;
        }
    }
    for (i = 0; i < size; i++) {
        for (j = 0; j <= i; j++) {
            printf("%d\n", pa[i][j]); 
        }
    }
    for(;;) {
        my_free(pa[--i]);
        if (!i) {
            break;
        }
    }
    return 0;
}