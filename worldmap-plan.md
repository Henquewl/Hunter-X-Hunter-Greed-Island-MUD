# Plano: Greed Island World Map

**Data da pesquisa:** Jun 23 2026  
**Status:** Pronto para implementação futura

---

## Contexto

O milestone v1.00 exige que o mundo seja remodelado para refletir canonicamente Greed Island (Antokiba, Masadora, Rabicuta, Limeiro Castle, etc.), substituindo o conteúdo genérico TbaMUD. A peça central é um mapa-mundi navegável da ilha, que requer centenas de salas conectadas em grid — muito além do limite convencional de 100 salas por zona.

---

## Decisão de design: 256×256 com 4 quadrantes (inspirado no FE MUD DragonBall Z)

O usuário quer o mesmo modelo do FE MUD: coordenadas de 1,1 a 128,128 e de -128,-128 a -1,-1 (4 quadrantes, sem origem em 0,0). Total: 256×256 = **65.536 salas**.

### Por que isso exige mudar o tipo de vnum

`room_vnum = ush_int` (unsigned short, 2 bytes, max 65.535). `NOWHERE = 65.535` é sentinel reservado. Máximo de vnums usáveis no jogo inteiro: **65.534**. 256×256 = 65.536 — impossível sem ampliar o tipo.

### Custo real da mudança de IDXTYPE

Auditoria feita no código (`grep %hd %hu (ush_int)` em `src/`):
- **5 ocorrências em 2 arquivos**: `src/db.c` (4) e `src/shop.c` (1).
- Player files, object saves e world files são **todos formato ASCII** → não são afetados por mudança de tipo binário.
- O typedef `ush_int` permanece no código (usado em outras structs); só `IDXTYPE` muda.

**Conclusão: a mudança de IDXTYPE é a pré-condição mais simples possível** — 5 linhas a corrigir manualmente após alterar `structs.h`.

---

## Achados da pesquisa

### Sistema de zonas (sem alteração no engine necessária)

- O limite de 100 salas/zona é **convenção, não regra do engine**. A única validação em `db.c` é `bot > top`. Uma zona com 900 salas (bot=42000, top=42899) carrega sem qualquer modificação de código.
- `real_room()` usa busca binária no array `world[]` — sem lógica aritmética que assuma tamanho de zona.
- Teto de vnum: **65.534** (65.535 = `NOWHERE` sentinel). Zonas 411–419 estão todas vazias, espaço limpo disponível.
- **Confirmado pelo fórum tbamud.com**: "you can create a zone of any size you want."

### Infraestrutura worldmap existente (`src/asciimap.c`)

Já existe um engine de mapa ASCII completo com dois modos:

| Modo | Tiles | Conectores de saída | Quando ativo |
|---|---|---|---|
| Normal | `[X]` 3 chars, com cor | Sim (`|`, `-`) | Padrão |
| Worldmap | 1 char bare (`·`, `~`, etc.) | Não | XOR de flags (ver abaixo) |

**Lógica XOR em `show_worldmap()` (`asciimap.c:774`):**
- `ZONE_WORLDMAP` setado, sala SEM `ROOM_WORLDMAP` individual → **modo worldmap** ✓
- `ZONE_WORLDMAP` setado, sala COM `ROOM_WORLDMAP` individual → **modo normal** (útil para entradas de cidades)
- Nenhum dos dois ou ambos → modo normal

**Zonas GI já usando worldmap** (não mexer):
- Zona 400 (40000–40099): Greed Island Start — `ZONE_WORLDMAP`
- Zona 401 (40100–40199): Path to G.I. southern — `ZONE_WORLDMAP`
- Zona 402 (40200–40299): Road to Ai Ai — salas individuais com `ROOM_WORLDMAP`
- Zona 410 (41000–41099): Dorias/Landing Platform — salas individuais com `ROOM_WORLDMAP`

**Canvas do mapa**: `MAX_MAP_SIZE = 12` (raio máximo visível), canvas `51×51`. Com grid 256×256, um jogador no centro vê até 12 salas em todas as direções — ilha com margens visíveis.

### Pesquisa tbamud.com (fórum)

- O [Luminari MUD](https://luminarimud.com) implementou wilderness **1024×1024** usando um pool dinâmico de vnums. Abordagem descartada para cá: pool dinâmico causa bugs de persistência (jogador desconecta e reconecta em sala errada porque vnum foi reatribuído). Nossa ilha é estática — sem esse problema.
- Criar grids grandes manualmente com buildwalk/OLC é descrito como "tedioso e propenso a erros". **Conclusão unânime no fórum: usar script de geração de arquivos de texto** (.wld/.zon), não o OLC interativo.

---

## Nota sobre 255×255

255×255 = 65.025 salas — ainda não resolve. O maior bloco contíguo livre no espaço de vnums atual é ~24.000 (41100–65299). 65.025 não cabe. O problema não é o total de vnums: é que o mapa precisa de um bloco contíguo, e não existe um dessa magnitude sem a mudança de tipo.

Além disso: 12.713 (salas existentes) + 65.025 (255×255) = 77.738 vnums necessários no total. Teto disponível: 65.534. Impossível mesmo reorganizando tudo.

---

## Abordagem recomendada: IDXTYPE → uint32_t + Zona 1000 (256×256)

### Passo 0 (pré-condição): Ampliar IDXTYPE para uint32_t

**Por que é necessário:** Qualquer grid acima de ~155×155 exige bloco contíguo de vnums que não existe no espaço atual (ush_int, max 65.534).

**Por que é seguro:**
- Todos os world files, player files e object saves usam **formato ASCII** (não binário de 2 bytes) → não quebra saves existentes.
- `ush_int` typedef permanece no código para outras structs; apenas `IDXTYPE` muda.
- Apenas **5 ocorrências** de `%hd`/`%hu`/`(ush_int)` em `src/db.c` (4) e `src/shop.c` (1) — as únicas linhas a corrigir após a mudança.
- Com `uint32_t`, `NOWHERE = 0xFFFFFFFF`. Toda a lógica existente continua funcionando.

**Mudança em `structs.h`** (1 linha):
```c
// Antes:
#define IDXTYPE  ush_int
// Depois:
#define IDXTYPE  uint32_t
```

Após isso, compilar e corrigir os 5 `%hd` → `%u`.

### Especificação da zona mundial

- **Zona 1000**, vnums **100.000–165.535** (65.536 salas = 256×256)
- Flag da zona: `g` (bit 6 = `ZONE_WORLDMAP`) na linha de header do .zon
- Fórmula de vnum: `vnum = 100000 + (row × 256) + col` onde row ∈ [0–255], col ∈ [0–255]
- **Coordenadas player-facing** (exibidas no prompt/look, estilo FE MUD):
  - `x = col - 128` → range [-128, 127] (negativo = oeste, positivo = leste)
  - `y = 128 - row` → range [-128, 127] (negativo = sul, positivo = norte)
  - Quadrante I: x ∈ [1,128], y ∈ [1,128] / Quadrante III: x ∈ [-128,-1], y ∈ [-128,-1]
- **Borda oceânica**: linhas 0–1 e 254–255 + colunas 0–1 e 254–255 → `SECT_WATER_NOSWIM`, sem exits externos
- **Cidades**: salas de entrada usam AMBAS as flags → XOR desliga worldmap (exibe como sala normal), válidas para teleporte

### Localização das cidades no grid (aprox. — a definir no layout)

| Cidade | x,y (player) | row, col | Vnum aprox. |
|---|---|---|---|
| Antokiba (cidade inicial) | 0, 0 | 128, 128 | 132.896 |
| Masadora (spell cards) | 50, 40 | 88, 178 | 122.706 |
| Rabicuta | -40, -30 | 158, 88 | 140.568 |
| Limeiro Castle | 0, 60 | 68, 128 | 117.504 |
| Porto/Dorias | -10, -60 | 188, 118 | 148.086 |

*(coordenadas exatas a definir na implementação com base no mapa canônico da ilha)*

---

## Conflitos e como evitá-los

| Risco | Localização | Resolução |
|---|---|---|
| Zonas existentes (0–654) | Todas | Zona 1000 começa em vnum 100.000 — acima de tudo que existe. Zero sobreposição. |
| Teleporte de cartas (`act.item.c:1416`) | Escaneia range da zona da cidade, não da zona 1000 | Salas do grid não têm `ROOM_WORLDMAP` individual → não são alvos de teleporte. Entradas de cidades têm ambas as flags → são alvos automáticos. Sem mudança. |
| Lógica XOR de worldmap (`asciimap.c:774`) | Zonas 400/401/402/410 | Não alteramos essas zonas; zona 1000 é novo `ZONE_WORLDMAP` independente |
| Saída de zona 410, sala 41001 | `lib/world/wld/410.wld` | **Um exit a alterar**: `D1 (north)` muda de `40060` para o vnum da entrada do porto no grid (~148.086). Única edição em arquivo existente. |
| `%hd` format strings | `src/db.c` (4×), `src/shop.c` (1×) | Mudar para `%u` nos 5 casos após alterar `IDXTYPE`. Trivial. |
| `city_met[8]` overflow (`structs.h:1018`) | `act.item.c:1487` | Não marcar cidades GI com `ZONE_CITY` ainda — tarefa separada |
| `genzon.c` limit 655 | `genzon.c:67` | OLC vai rejeitar zona 1000 via interface interativa. Não usar OLC para criar/editar a zona — apenas editar arquivos de texto diretamente. |
| Índices dos world files | `lib/world/*/index` | Adicionar entrada `1000` em ordem numérica em todos os 7 índices |

---

## Arquivos a criar/modificar

### Engine (mudança mínima de tipo)
```
src/structs.h             <- 1 linha: IDXTYPE ush_int -> uint32_t
src/db.c                  <- 4 ocorrencias de %hd -> %u (vnum printf)
src/shop.c                <- 1 ocorrencia de %hd -> %u (vnum printf)
```

### Novos (todos criados pelo script de geração)
```
lib/world/zon/1000.zon    <- header da zona: bot=100000, top=165535, flag=g (ZONE_WORLDMAP)
lib/world/wld/1000.wld    <- 65.536 salas com exits N/S/E/W wired
lib/world/mob/1000.mob    <- vazio ($)
lib/world/obj/1000.obj    <- vazio ($)
lib/world/shp/1000.shp    <- vazio ($~)
lib/world/qst/1000.qst    <- vazio ($~)
lib/world/trg/1000.trg    <- vazio ($~)
lib/world/map/greed_island.txt  <- fonte canonica do mapa (256 linhas x 256 chars)
```

### Modificados manualmente (mínimo)
```
lib/world/wld/410.wld     <- alterar 1 exit (sala 41001 D1: ~148086)
lib/world/*/index         <- adicionar "1000" em cada um dos 7 indices
```

### Sem alterações
```
src/asciimap.c            <- nao tocar (worldmap renderer ja funciona)
src/act.item.c            <- nao tocar (teleporte funciona por flags)
```

---

## Exibição de coordenadas para o jogador

Para reproduzir a experiência do FE MUD (jogador vê "Você está em (42, -15)" em vez de "Sala 135970"):

Adicionar ao `look` (em `act.informative.c`) uma linha de coordenadas quando o jogador está em zona com `ZONE_WORLDMAP`:

```c
if (ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(ch)), ZONE_WORLDMAP)) {
    room_vnum v = GET_ROOM_VNUM(IN_ROOM(ch));
    int col = (v - 100000) % 256;
    int row = (v - 100000) / 256;
    int x = col - 128;
    int y = 128 - row;
    send_to_char(ch, "[ Coordenadas: %d, %d ]\r\n", x, y);
}
```

Esta é a única adição opcional ao engine — não é necessária para o mapa funcionar, mas melhora a UX.

---

## Script de geração (`tools/gen_worldmap.py`)

O script lê `lib/world/map/greed_island.txt` (256 linhas × 256 chars) e gera `1000.zon` e `1000.wld`. Requisitos:

1. **Fonte**: `greed_island.txt` — 256 linhas de 256 chars. Cada char = setor:
   ```
   . = SECT_FIELD (2)      ^ = SECT_MOUNTAIN (5)    ~ = SECT_WATER_NOSWIM (7)
   f = SECT_FOREST (3)     h = SECT_HILLS (4)        = = SECT_WATER_SWIM (6)
   # = SECT_CITY (1)       @ = cidade (entrada -- gera ROOM_WORLDMAP)
   ```
2. **Fórmula de vnum**: `vnum = 100000 + row * 256 + col`
3. **Exits**: cada sala conecta N/S/E/W às salas adjacentes, exceto borda (`~`) sem exits externos
4. **Salas `@` (entradas de cidade)**: flags incluem `65536` (`ROOM_WORLDMAP`), exit adicional para vnum da zona de cidade (configurável no script)
5. **Header .zon**: `100000 165535 30 2 g 0 0 0 -1 -1`
6. **Validação**: 65.536 salas, sem vnum duplicado, borda completa de `~`, exits de cidades válidos
7. **Desempenho**: o .wld gerado terá ~6–8 MB — normal para esse tamanho

---

## Setores do mapa (sugestões para o layout da ilha)

| Setor | Constante TbaMUD | Tile worldmap | Uso |
|---|---|---|---|
| 0 | `SECT_INSIDE` | marrom | cavernas internas |
| 1 | `SECT_CITY` | cinza | entradas de cidade |
| 2 | `SECT_FIELD` | verde claro | campos abertos |
| 3 | `SECT_FOREST` | verde escuro | florestas |
| 4 | `SECT_HILLS` | amarelo | colinas |
| 5 | `SECT_MOUNTAIN` | branco | montanhas |
| 6 | `SECT_WATER_SWIM` | azul claro | rios/lagos |
| 7 | `SECT_WATER_NOSWIM` | azul escuro | oceano/borda |

---

## Ordem de implementação (para a sessão futura)

### Fase 1 — Engine (pré-condição, ~30min)
1. `structs.h`: mudar `IDXTYPE` de `ush_int` para `uint32_t`
2. Corrigir 5 printf: `%hd` → `%u` em `db.c` (4×) e `shop.c` (1×)
3. Compilar: `cd src && make circle CFLAGS=-w` — deve compilar sem erros
4. Boot rápido e verificar zonas existentes ainda funcionam

### Fase 2 — Layout do mapa (design, fora do código)
5. Desenhar `greed_island.txt` (256×256) inspirado no mapa canônico da ilha
   - Usar editor de texto simples; cada char = um tile
   - Definir posições das 5+ cidades principais (char `@`)
   - Borda: primeiras/últimas 2 linhas e colunas = `~`

### Fase 3 — Script de geração
6. Escrever `tools/gen_worldmap.py` que lê o .txt e gera `1000.zon` + `1000.wld`
7. Executar, validar saída (65.536 rooms, exits corretos)

### Fase 4 — Integração
8. Copiar arquivos gerados para `lib/world/zon/` e `lib/world/wld/`
9. Criar arquivos vazios: `1000.mob`, `1000.obj`, `1000.shp`, `1000.qst`, `1000.trg`
10. Adicionar `1000` em todos os 7 `lib/world/*/index` (em ordem numérica — no final)
11. Editar `410.wld` sala 41001: exit D1 → vnum do porto no grid
12. Boot e teste: `goto 132896` (Antokiba no grid), `map` e `map world`, andar para borda, confirmar oceano
13. Verificar card de teleporte em zona 410 ainda pousa em 41000 (não no grid)

---

## Referências

- Fórum tbamud.com — [Virtual Wilderness, Room Pools and coordinate confusion](https://www.tbamud.com/forums/4-development/3601-virtual-wilderness-room-pools-and-coordinate-confusion)
- Fórum tbamud.com — [Creating large grid zones](https://www.tbamud.com/kunena/3-building/3984-creating-large-grid-zones)
- `src/asciimap.c` — renderer existente: `show_worldmap()` (XOR lógica), `MapArea()`, `WorldMap()`
- `src/act.item.c:1416,1676` — lógica de teleporte por cartas (referência de conflito)
- `src/structs.h:38-44` — definição de `IDXTYPE`, `CIRCLE_UNSIGNED_INDEX`, `NOWHERE`
- `src/db.c` e `src/shop.c` — únicas 5 ocorrências de `%hd`/`%hu` a corrigir
