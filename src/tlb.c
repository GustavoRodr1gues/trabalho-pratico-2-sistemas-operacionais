#include "tlb.h"
#include "config.h"

/*
 * Array de entradas da TLB. Cada entrada guarda o número da página lógica,
 * o frame físico correspondente e um bit de validade.
 */
static tlb_entry_t tlb[TLB_SIZE];

/*
 * Índice da próxima posição a ser sobrescrita na inserção.
 * Avança circularmente a cada inserção, implementando FIFO: a entrada mais
 * antiga é sempre a próxima a ser substituída, independentemente de haver
 * slots livres no array.
 */
static int fifo_next = 0;

/* Inicializa todas as entradas como inválidas e reseta o ponteiro FIFO. */
void tlb_init(void)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page  = -1;
        tlb[i].frame = -1;
        tlb[i].valid = 0;
    }

    fifo_next = 0;
}

/*
 * Busca a página no TLB e retorna o frame físico associado.
 * Retorna -1 se a página não estiver presente (TLB miss).
 */
int tlb_lookup(int page)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            return tlb[i].frame;
        }
    }

    return -1;
}

/*
 * Insere ou atualiza o mapeamento (page → frame) na TLB.
 *
 * Se a página já existir em alguma entrada válida, apenas o frame é
 * atualizado — o ponteiro FIFO não avança, preservando a ordem de entrada.
 *
 * Caso contrário, a entrada apontada por fifo_next é sobrescrita (seja ela
 * livre ou ocupada) e o ponteiro avança circularmente. Isso garante que a
 * ordem de saída reflita estritamente a ordem de chegada (FIFO puro), mesmo
 * quando tlb_remove() invalida slots no meio do array.
 */
void tlb_insert(int page, int frame)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            tlb[i].frame = frame;
            return;
        }
    }

    tlb[fifo_next].page  = page;
    tlb[fifo_next].frame = frame;
    tlb[fifo_next].valid = 1;

    fifo_next = (fifo_next + 1) % TLB_SIZE;
}

/*
 * Invalida a entrada da página informada, sem alterar o ponteiro FIFO.
 * Chamada ao expulsar uma página da memória física para garantir que
 * a TLB não retorne um frame obsoleto.
 */
void tlb_remove(int page)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            tlb[i].valid = 0;
        }
    }
}

/* Reseta a TLB para o estado inicial (equivale a tlb_init). */
void tlb_clear(void)
{
    tlb_init();
}
