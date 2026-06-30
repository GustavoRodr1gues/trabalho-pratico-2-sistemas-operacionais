#include <stdio.h>

#include "statistics.h"

static int total_addresses = 0;
static int page_faults     = 0;
static int tlb_hits        = 0;

/* Zera todos os contadores. Deve ser chamada antes do loop principal. */
void statistics_init(void)
{
    total_addresses = 0;
    page_faults     = 0;
    tlb_hits        = 0;
}

/* Incrementa o total de endereços traduzidos (chamada a cada iteração). */
void count_address(void)
{
    total_addresses++;
}

/* Incrementa o contador de page faults. */
void count_page_fault(void)
{
    page_faults++;
}

/* Incrementa o contador de TLB hits. */
void count_tlb_hit(void)
{
    tlb_hits++;
}

int get_total_addresses(void)
{
    return total_addresses;
}

int get_page_faults(void)
{
    return page_faults;
}

int get_tlb_hits(void)
{
    return tlb_hits;
}

/*
 * Imprime as estatísticas acumuladas ao final da simulação:
 *   - Total de endereços traduzidos
 *   - Número absoluto e taxa de page faults  (page_faults / total)
 *   - Número absoluto e taxa de TLB hits     (tlb_hits / total)
 */
void print_statistics(void)
{
    double page_fault_rate = 0.0;
    double tlb_hit_rate    = 0.0;

    if (total_addresses > 0) {
        page_fault_rate = (double) page_faults / total_addresses;
        tlb_hit_rate    = (double) tlb_hits    / total_addresses;
    }

    printf("Number of Translated Addresses = %d\n", total_addresses);
    printf("Page Faults = %d\n",          page_faults);
    printf("Page Fault Rate = %.3f\n",    page_fault_rate);
    printf("TLB Hits = %d\n",             tlb_hits);
    printf("TLB Hit Rate = %.3f\n",       tlb_hit_rate);
}
