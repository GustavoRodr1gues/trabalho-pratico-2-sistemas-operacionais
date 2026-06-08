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
         * O arquivo de entrada possui inteiros de 32 bits.
         * Apenas os 16 bits menos significativos devem ser considerados.
         * TODO: Implementar lógica de obtenção do número da página 
         * e do offset.
         */
        logical_address = 0;
        int page = 0;
        int offset = 0;

        int frame = tlb_lookup(page);

        if (frame != -1) {
            count_tlb_hit();
        } else {
            frame = page_table_lookup(page);

            if (frame == -1) {
                count_page_fault();
                frame = handle_page_fault(page);
            }

            tlb_insert(page, frame);
        }

        /*
         * TODO:
         * A política de atualização do LRU aproximado pode ser ajustada.
         * Nesta versão-base, a página acessada é marcada como referenciada
         * e o aging é atualizado a cada acesso.
         */

        page_table_set_reference(page);
        page_table_update_aging();
        
        /*
        * TODO: Implementar cálculo do physical_address.
        */
        int physical_address = 0;
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
