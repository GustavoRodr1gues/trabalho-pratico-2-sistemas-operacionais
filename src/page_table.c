#include "page_table.h"
#include "config.h"

/*
 * Tabela de páginas linear com uma entrada por página lógica.
 * Cada entrada mantém o frame físico, o bit de validade e os campos
 * usados pelo algoritmo de LRU aproximado (aging).
 */
static page_table_entry_t page_table[PAGE_TABLE_SIZE];

/* Inicializa todas as entradas como inválidas com contadores zerados. */
void page_table_init(void)
{
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i].frame           = -1;
        page_table[i].valid           = 0;
        page_table[i].reference_bit   = 0;
        page_table[i].aging_counter   = 0;
    }
}

/*
 * Consulta a tabela de páginas para a página informada.
 * Retorna o frame físico se a entrada for válida, ou -1 caso contrário
 * (indicando que a página não está carregada em memória — page fault).
 */
int page_table_lookup(int page)
{
    if (page_table[page].valid) {
        return page_table[page].frame;
    }

    return -1;
}

/*
 * Registra o mapeamento page → frame na tabela de páginas, marcando a
 * entrada como válida. O reference_bit e o aging_counter são zerados
 * para que a página recém-carregada comece com histórico limpo.
 */
void page_table_update(int page, int frame)
{
    page_table[page].frame         = frame;
    page_table[page].valid         = 1;
    page_table[page].reference_bit = 0;
    page_table[page].aging_counter = 0;
}

/*
 * Invalida a entrada da página, indicando que ela não está mais em memória.
 * Chamada quando o frame da página é reutilizado para outra página (substituição).
 */
void page_table_invalidate(int page)
{
    page_table[page].frame         = -1;
    page_table[page].valid         = 0;
    page_table[page].reference_bit = 0;
    page_table[page].aging_counter = 0;
}

/*
 * Seta o reference_bit da página como 1, indicando que ela foi acessada
 * no intervalo de tempo atual. Esse bit é consumido por page_table_update_aging().
 */
void page_table_set_reference(int page)
{
    page_table[page].reference_bit = 1;
}

/*
 * Atualiza os contadores de aging de todas as páginas válidas.
 *
 * Algoritmo de aging (LRU aproximado):
 *   1. Desloca o aging_counter 1 bit à direita (envelhecendo os acessos passados).
 *   2. Se reference_bit == 1, seta o MSB do counter (0x80), registrando que
 *      a página foi acessada neste intervalo.
 *   3. Zera o reference_bit para o próximo intervalo.
 *
 * Após N chamadas, os N bits mais significativos de aging_counter representam
 * o histórico de acessos dos últimos N intervalos. A página com o menor counter
 * é a menos referenciada recentemente e será escolhida como vítima.
 */
void page_table_update_aging(void)
{
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {

        if (!page_table[i].valid) {
            continue;
        }

        page_table[i].aging_counter >>= 1;

        if (page_table[i].reference_bit) {
            page_table[i].aging_counter |= 0x80;
        }

        page_table[i].reference_bit = 0;
    }
}

/* Retorna o frame físico da página, ou -1 se o índice for inválido. */
int page_table_get_frame(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return -1;
    }

    return page_table[page].frame;
}

/* Retorna 1 se a página está presente em memória física, 0 caso contrário. */
int page_table_is_valid(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return 0;
    }

    return page_table[page].valid;
}

/* Retorna o aging_counter da página, usado por select_victim_page(). */
unsigned char page_table_get_aging_counter(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return 0;
    }

    return page_table[page].aging_counter;
}
