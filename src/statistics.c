#include <stdio.h>

#include "statistics.h"

static int total_addresses = 0;
static int page_faults = 0;
static int tlb_hits = 0;

void statistics_init(void)
{
    total_addresses = 0;
    page_faults = 0;
    tlb_hits = 0;
}

void count_address(void)
{
    /*TODO: Implementar contador total_addresses*/
}

void count_page_fault(void)
{
    /*TODO: Implementar contador page_faults*/
}

void count_tlb_hit(void)
{
    
    /*TODO: Implementar contador tlb_hits*/
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

void print_statistics(void)
{
    double page_fault_rate = 0.0;
    double tlb_hit_rate = 0.0;

    /*
     * TODO: Implementar cálculo do page_fault_rate e tlb_hit_rate
    */

    printf("Number of Translated Addresses = %d\n", total_addresses);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", page_fault_rate);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", tlb_hit_rate);
}
