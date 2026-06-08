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
    /*
     * TODO:
     * 1. Procurar quadro livre.
     * 2. Se não houver quadro livre, selecionar página vítima.
     * 3. Invalidar página vítima na tabela de páginas.
     * 4. Remover página vítima do TLB.
     * 5. Ler a página correta do BACKING_STORE.bin.
     * 6. Atualizar frame_to_page.
     * 7. Atualizar tabela de páginas.
     * 8. Retornar número do frame.
     */

    int frame = find_free_frame();

    if (frame == -1) {
        int victim_page = select_victim_page();

        /*
         * TODO:
         * Obter o quadro da página vítima.
         * Invalidar tabela e TLB.
         */

        (void) victim_page;

        frame = 0;
    }

    /*
     * TODO:
     * Fazer fseek para page * PAGE_SIZE.
     * Fazer fread de PAGE_SIZE bytes para physical_memory[frame].
     */

    if (backing == NULL) {
        fprintf(stderr, "Erro interno: BACKING_STORE nao inicializado.\n");
        exit(1);
    }

    (void) page;

    return frame;
}

int select_victim_page(void)
{
    /*
     * TODO:
     * Selecionar a página válida com menor aging_counter.
     * Em caso de empate, qualquer critério consistente pode ser usado.
     */

    return 0;
}

signed char read_memory(int frame, int offset)
{
    /*
     * TODO:
     * Retornar o byte armazenado em physical_memory[frame][offset].
     */

    (void) frame;
    (void) offset;
    return 0;
}

int get_page_loaded_in_frame(int frame)
{
    if (frame < 0 || frame >= NUM_FRAMES) {
        return -1;
    }

    return frame_to_page[frame];
}
