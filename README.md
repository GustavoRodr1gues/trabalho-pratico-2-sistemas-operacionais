# Trabalho Prático 2 — Gerenciador de Memória Virtual

Simulador de gerência de memória virtual implementado em C. O programa lê endereços lógicos da entrada padrão, realiza a tradução para endereços físicos utilizando TLB, tabela de páginas com substituição por LRU aproximado e uma _backing store_ em disco, e imprime o endereço físico e o valor armazenado para cada acesso.

---

## Estrutura do projeto

```
.
├── Makefile
├── README.md
├── include/
│   ├── config.h        # Constantes globais (tamanhos de página, frames, TLB)
│   ├── tlb.h           # Interface da TLB
│   ├── page_table.h    # Interface da tabela de páginas
│   ├── memory.h        # Interface da memória física
│   └── statistics.h    # Interface das estatísticas
├── src/
│   ├── main.c          # Loop principal de tradução de endereços
│   ├── tlb.c           # TLB com substituição FIFO (16 entradas)
│   ├── page_table.c    # Tabela de páginas com LRU aproximado (aging)
│   ├── memory.c        # Memória física e tratamento de page fault
│   └── statistics.c    # Contadores e cálculo de taxas
├── data/
│   ├── generate_data.py    # Gera os arquivos de entrada e a backing store
│   ├── BACKING_STORE.bin   # Memória secundária (64 KB)
│   ├── addresses_random.txt    # 10.000 endereços aleatórios
│   └── addresses_location.txt  # 10.000 endereços com localidade
└── report/             # Coloque aqui o relatório final em PDF
```

---

## Parâmetros do sistema

| Parâmetro | Valor | Descrição |
|---|---|---|
| Tamanho da página | 256 bytes | 8 bits de offset |
| Entradas na tabela de páginas | 256 | 8 bits de número de página |
| Número de frames físicos | 128 | Memória física = 32 KB |
| Entradas na TLB | 16 | Substituição FIFO |
| Tamanho da backing store | 64 KB | 256 páginas × 256 bytes |

---

## Pré-requisitos

- GCC (ou MinGW/MSYS2 no Windows)
- Python 3 (apenas para gerar os dados de entrada)
- Make

---

## Configuração inicial (apenas uma vez)

Gere a backing store e os arquivos de endereços antes da primeira execução:

```bash
cd data
python3 generate_data.py
cd ..
```

Isso cria:
- `data/BACKING_STORE.bin` — 64 KB com valores byte `i % 256` na posição `i`
- `data/addresses_random.txt` — 10.000 endereços aleatórios (seed 42)
- `data/addresses_location.txt` — 10.000 endereços com localidade espacial/temporal (seed 42)

---

## Compilação

```bash
make
```

Para limpar o binário:

```bash
make clean
```

---

## Execução

### Usando o Makefile

```bash
# Compila e executa com endereços aleatórios
make run-random

# Compila e executa com endereços de localidade
make run-location
```

### Manualmente

```bash
# Endereços aleatórios
./vm < data/addresses_random.txt

# Endereços com localidade
./vm < data/addresses_location.txt

# Ou redirecionar a saída para um arquivo
./vm < data/addresses_random.txt > saida.txt
```

> **Windows (cmd/PowerShell):** substitua `./vm` por `.\vm.exe`

---

## Formato de saída

Cada linha de acesso imprime:

```
Logical address: <endereço_lógico> Physical address: <endereço_físico> Value: <valor>
```

Ao final, as estatísticas globais:

```
Number of Translated Addresses = 10000
Page Faults = 244
Page Fault Rate = 0.024
TLB Hits = 5415
TLB Hit Rate = 0.542
```

---

## Arquitetura e algoritmos implementados

### Tradução de endereços

O endereço lógico de entrada é um inteiro de 32 bits. Apenas os 16 bits menos significativos são considerados:

```
bits 15–8  →  número da página  (8 bits → até 256 páginas)
bits  7–0  →  offset dentro da página  (8 bits → 256 bytes)
```

O endereço físico é calculado como:

```
endereço físico = (frame << 8) | offset
```

### Pipeline de tradução (main.c)

Para cada endereço lógico:

1. Consulta a **TLB** — se encontrado (_TLB hit_), obtém o frame diretamente
2. Consulta a **tabela de páginas** — se válido, obtém o frame e insere na TLB
3. Se inválido, dispara um **page fault**: carrega a página da backing store para um frame livre (ou para o frame da vítima LRU), atualiza a tabela de páginas e a TLB
4. Marca a página como referenciada e atualiza os contadores de aging de todas as páginas

### TLB — Translation Lookaside Buffer (tlb.c)

- Array circular de 16 entradas (`tlb_entry_t { page, frame, valid }`)
- **Política de substituição: FIFO** — o ponteiro `fifo_next` avança a cada inserção; a posição apontada é sempre sobrescrita, seja ela livre ou ocupada
- Ao invalidar um frame (page fault com substituição), a entrada correspondente é removida da TLB para manter a consistência

### Tabela de páginas (page_table.c)

- Array de 256 entradas (`page_table_entry_t { frame, valid, reference_bit, aging_counter }`)
- Cada entrada rastreia o frame físico associado, se a página está presente em memória e o histórico de acessos via aging counter

### LRU Aproximado por Aging (page_table.c + memory.c)

A substituição de páginas usa o algoritmo de **aging**:

- A cada acesso, o `reference_bit` da página acessada é setado como `1`
- Após cada acesso (`page_table_update_aging`), para **todas** as páginas válidas:
  - O `aging_counter` é deslocado 1 bit à direita: `counter >>= 1`
  - Se `reference_bit == 1`, o bit mais significativo é setado: `counter |= 0x80`
  - O `reference_bit` é zerado
- Na substituição (`select_victim_page`), a página com o **menor** `aging_counter` é escolhida como vítima — ela foi menos acessada recentemente

Exemplo com 8 períodos:

```
Período:     8  7  6  5  4  3  2  1   → aging_counter
Página A:    1  1  0  0  1  0  0  1   → 0b11001001 = 0xC9  (mais recente)
Página B:    0  0  1  1  0  0  0  0   → 0b00110000 = 0x30  (menos recente → vítima)
```

### Tratamento de page fault (memory.c)

1. Procura um frame livre (`find_free_frame`)
2. Se não há frame livre, seleciona a vítima pelo LRU aproximado (`select_victim_page`)
3. Invalida a vítima na tabela de páginas e na TLB
4. Lê a página solicitada da backing store com `fseek(page * PAGE_SIZE)` + `fread`
5. Atualiza `frame_to_page` e a tabela de páginas

---

## Interpretando os resultados

| Arquivo de entrada | Comportamento esperado |
|---|---|
| `addresses_random.txt` | Alta taxa de page fault (~24%), TLB hit moderado (~54%) |
| `addresses_location.txt` | Baixa taxa de page fault, TLB hit elevado (>90%) |

A diferença ilustra o impacto da **localidade de referência** nas hierarquias de memória: endereços com localidade reutilizam páginas e entradas de TLB com muito mais frequência.
