# ProgramScheduler

## Descrição

Programa para escalonar a execução postergada de outros
programas multi-processo. Permite a configuração do
tempo mínimo de atraso da execução de um programa e da
topologia utilizada pelo escalonador de processos.
Trabalho final da disciplina de Sistemas Operacionais
2019/1 da Universidade de Brasília.

## Integrantes

Nome                     | Matrícula
------------------------ | ----------
André Laranjeira         | 16/0023777
Hugo Nascimento Fonseca  | 16/0008166
José Luiz Gomes Nogueira | 16/0032458
Victor André Gris Costa  | 16/0019311

## Ambiente de execução esperado

* Ambiente Linux com suporte a filas de mensagens

## Instruções de compilação

1. Entre na pasta raiz do projeto
2. Execute o comando `cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=<Debug ou Release>`
3. Execute `make [nome do executável]` para compilar um
   executável específico ou `make all` para compilar
   todos os executáveis.

## Modo de uso

1. Inicie o escalonador a partir da pasta raiz do projeto
   com `./scheduler <nome da topologia>` sendo que
   `<nome da topologia>` pode ser `tree`, `torus` e
   `hypercube`
2. Adicione programas para execução com o comando
   `./execute <caminho para programa> <argumentos opcionais> <espera em segundos>`. O caminho pode ser
   relativo ou absoluto a espera deve ser um número
   inteiro.
3. Finalize a execução do escalonador com o comando `./shutdown`.
   O escalonador irá finalizar assim que possível. Caso
   haja um programa em execução, ele irá aguardar a
   finalização desse programa e terminar sua execução.
