#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "tlb.h"
#include "page_table.h"
#include "memory.h"
#include "statistics.h"

int main(void)
{
    FILE *backing = fopen(BACKING_STORE_PATH, "rb");

    if (backing == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", BACKING_STORE_PATH);
        fprintf(stderr, "Execute antes: cd data && python3 generate_data.py\n");
        return 1;
    }

    page_table_init();
    tlb_init();
    memory_init(backing);
    statistics_init();

    int logical_address;

    while (scanf("%d", &logical_address) == 1) {
        count_address();

        /*
         * O arquivo de entrada contém inteiros de 32 bits.
         * Apenas os 16 bits menos significativos são usados:
         *   bits 15–8 → número da página  (8 bits, até 256 páginas)
         *   bits  7–0 → offset na página  (8 bits, 256 bytes por página)
         */
        logical_address = logical_address & 0xFFFF;
        int page   = (logical_address >> 8) & 0xFF;
        int offset =  logical_address & 0xFF;

        /*
         * Etapa 1: consulta a TLB. Em caso de hit, o frame é retornado
         * diretamente sem acessar a tabela de páginas.
         */
        int frame = tlb_lookup(page);

        if (frame != -1) {
            count_tlb_hit();
        } else {
            /*
             * Etapa 2: consulta a tabela de páginas. Retorna o frame
             * se a página estiver presente (bit válido = 1).
             */
            frame = page_table_lookup(page);

            if (frame == -1) {
                /*
                 * Etapa 3: page fault — a página não está em memória física.
                 * handle_page_fault() carrega a página da backing store,
                 * escolhe um frame livre ou expulsa a vítima LRU aproximado,
                 * e atualiza a tabela de páginas.
                 */
                count_page_fault();
                frame = handle_page_fault(page);
            }

            /* Mantém a TLB sincronizada após acesso via tabela de páginas. */
            tlb_insert(page, frame);
        }

        /*
         * Atualização do LRU aproximado (aging):
         * 1. Marca a página acessada como referenciada.
         * 2. Desloca os contadores de aging de todas as páginas válidas
         *    1 bit à direita e insere o reference_bit no MSB.
         * Isso permite que select_victim_page() estime qual página foi
         * menos acessada recentemente com base no menor aging_counter.
         */
        page_table_set_reference(page);
        page_table_update_aging();

        /*
         * Endereço físico: concatenação do frame (8 bits mais significativos)
         * com o offset (8 bits menos significativos).
         */
        int physical_address = (frame << 8) | offset;
        signed char value = read_memory(frame, offset);

        printf("Logical address: %d Physical address: %d Value: %d\n",
               logical_address,
               physical_address,
               value);
    }

    print_statistics();

    fclose(backing);

    return 0;
}
