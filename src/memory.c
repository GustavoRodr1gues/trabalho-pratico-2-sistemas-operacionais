#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "config.h"
#include "page_table.h"
#include "tlb.h"

/*
 * Memória física simulada: NUM_FRAMES frames de FRAME_SIZE bytes cada.
 * Total: 128 × 256 = 32 KB.
 */
static signed char physical_memory[NUM_FRAMES][FRAME_SIZE];

/*
 * Tabela reversa frame → página. Permite saber qual página lógica está
 * carregada em cada frame sem percorrer a tabela de páginas.
 * Valor -1 indica frame livre.
 */
static int frame_to_page[NUM_FRAMES];

/* Referência ao arquivo da backing store, aberto em main(). */
static FILE *backing = NULL;

/*
 * Inicializa a memória física zerando todos os bytes e marcando todos os
 * frames como livres. Armazena o ponteiro para a backing store.
 */
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

/*
 * Percorre frame_to_page em busca do primeiro frame livre (valor -1).
 * Retorna o índice do frame ou -1 se a memória estiver totalmente ocupada.
 */
static int find_free_frame(void)
{
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (frame_to_page[i] == -1) {
            return i;
        }
    }

    return -1;
}

/*
 * Trata um page fault para a página informada:
 *
 * 1. Tenta encontrar um frame livre. Se não houver, seleciona a página vítima
 *    pelo LRU aproximado (menor aging_counter), invalida sua entrada na tabela
 *    de páginas e na TLB, e reutiliza o frame liberado.
 * 2. Lê a página da backing store para o frame escolhido usando fseek/fread.
 *    O offset na backing store é page * PAGE_SIZE.
 * 3. Atualiza frame_to_page e a tabela de páginas com o novo mapeamento.
 *
 * Retorna o índice do frame onde a página foi carregada.
 */
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
        fprintf(stderr, "Erro interno: BACKING_STORE nao inicializado.\n");
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

/*
 * Seleciona a página vítima para substituição usando LRU aproximado.
 *
 * Percorre todas as páginas válidas e retorna aquela com o menor
 * aging_counter. Counters menores indicam que a página não foi referenciada
 * nos intervalos de tempo mais recentes, tornando-a a melhor candidata
 * para ser expulsa da memória física.
 */
int select_victim_page(void)
{
    int victim = -1;
    unsigned char smallest = 255;

    for (int page = 0; page < PAGE_TABLE_SIZE; page++) {

        if (!page_table_is_valid(page)) {
            continue;
        }

        unsigned char age = page_table_get_aging_counter(page);

        if (victim == -1 || age < smallest) {
            smallest = age;
            victim   = page;
        }
    }

    return victim;
}

/* Retorna o byte armazenado em physical_memory[frame][offset]. */
signed char read_memory(int frame, int offset)
{
    return physical_memory[frame][offset];
}

/*
 * Retorna o número da página lógica carregada no frame informado,
 * ou -1 se o frame estiver livre ou o índice for inválido.
 */
int get_page_loaded_in_frame(int frame)
{
    if (frame < 0 || frame >= NUM_FRAMES) {
        return -1;
    }

    return frame_to_page[frame];
}
