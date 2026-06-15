#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "config.h"
#include "page_table.h"
#include "tlb.h"

static signed char physical_memory[NUM_FRAMES][FRAME_SIZE];

/*
 * Indica qual página está carregada em cada quadro.
 * Valor -1 indica quadro livre.
 */
static int frame_to_page[NUM_FRAMES];

static FILE *backing = NULL;

void memory_init(FILE *backing_store)
{
    backing = backing_store;

    for (int i = 0; i < NUM_FRAMES; i++) {
        frame_to_page[i] = -1;

        for (int j = 0; j < FRAME_SIZE; j++) {
            physical_memory[i][j] = 0;
        }
    }
}

static int find_free_frame(void)
{
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (frame_to_page[i] == -1) {
            return i;
        }
    }

    return -1;
}

int handle_page_fault(int page)
{
    int frame = find_free_frame();

    if (frame == -1) {

        int victim_page = select_victim_page();

        frame = page_table_get_frame(victim_page);

        page_table_invalidate(victim_page);

        tlb_remove(victim_page);
    }

    if (backing == NULL) {
        fprintf(stderr,
                "Erro interno: BACKING_STORE nao inicializado.\n");
        exit(1);
    }

    fseek(backing, page * PAGE_SIZE, SEEK_SET);

    fread(physical_memory[frame],
          sizeof(signed char),
          PAGE_SIZE,
          backing);

    frame_to_page[frame] = page;

    page_table_update(page, frame);

    return frame;
}

int select_victim_page(void)
{
    int victim = -1;
    unsigned char smallest = 255;

    for (int page = 0; page < PAGE_TABLE_SIZE; page++) {

        if (!page_table_is_valid(page))
            continue;

        unsigned char age =
            page_table_get_aging_counter(page);

        if (victim == -1 || age < smallest) {
            smallest = age;
            victim = page;
        }
    }

    return victim;
}

signed char read_memory(int frame, int offset)
{
    return physical_memory[frame][offset];
}

int get_page_loaded_in_frame(int frame)
{
    if (frame < 0 || frame >= NUM_FRAMES) {
        return -1;
    }

    return frame_to_page[frame];
}