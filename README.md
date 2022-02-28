# Projeto ITP - magnus

"magnus" é a versão final do bot avançado de buraco para o projeto de ITP.

## Organização

O código do programa foi dividido nos seguintes arquivos:

* macros.h - Definição dos macros
* engine.h - Declaração dos tipos e das funções base
* engine.c - Código das funções de engine.h
* drawdiscard.h  - Declaração das funções de compra e descarte de cartas
* drawdiscard.c  - Código das funções de ckpt1.h
* meldnewjoin.h  - Declaração das funções de manipulação de jogos
* meldnewjoin.c  - Código das funções de ckpt2.h
* main.c - Código do main() e funções relacionadas

## Compilação

Execute o script "compileall":

```
chmod +x compileall
./compileall
```

Ou digite os seguintes comandos:

```
gcc -Wall engine.c drawdiscard.c meldnewjoin.c main.c -o magnus
gcc -Wall engine.c drawdiscard.c meldnewjoin.c main.c -o karpov
```

## Execução

Após compilado, execute o programa para iniciar o jogo com o comando:

```
node buraco.js magnus [bot2]
```

"[bot2]" pode ser um outro bot, ele próprio, ou vazio, caso o
usuário deseje jogar.

O script "autobattle" executa 100 partidas "magnus x karpov" (karpov é
uma cópia de magnus):

```
chmod +x autobattle
./autobattle
```

## Estratégia

### Compra de cartas:

O bot, sempre que possível, compra a carta do topo da pilha de descarte.
A carta comprada faz um grupo com as primeiras duas outras cartas que o
bot encontrar para fazer um jogo. Caso contrário, simplesmente compra
uma carta do baralho.

### Anexando jogos:

O bot nunca anexa cartas a jogos que possuam sete cartas ou mais, a
não ser que a mão seja composta somente por melés.

Na primeira etapa, o bot anexa qualquer carta que não seja um melé ao
primeiro jogo válido que encontrar.

Na segunda etapa, o bot então tenta anexar melés a jogos limpos que
possuam o mesmo naipe do melé. Se não houverem jogos com essa condição,
então o bot tenta anexá-los ao maior jogo limpo que encontrar. E se não
houverem jogos limpos, nada é anexado.

Esse processo se repete até não haverem mais cartas anexáveis na mão.

### Baixando jogos:

O bot baixa o primeiro jogo que encontrar na mão, independentemente do
tamanho do jogo. Esse processo se repete até nção haverem mais jogos
baixáveis na mão.

### Descarte de cartas:

Na primeira etapa, o bot descarta qualquer carta duplicada da mão. Também
descarta qualquer carta cujo naipe nenhuma outra carta na mão possua.

Na segunda etapa, o bot então descarta a carta que estiver mais distante
de fazer um jogo completo na mão.

Existem dois tipos de carta que o bot evita ao máximo descartar: melés
e cartas que o bot sabe que seu oponente poderá pegar do topo da pilha.
O primeiro é apenas descartado caso só hajam melés na mão, e o segundo
só é considerado para descarte na segunda etapa. 
