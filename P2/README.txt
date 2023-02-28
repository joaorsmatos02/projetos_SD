Projeto desenvolvido por:

Tomás Barreto nº 56282
João Matos nº 56292
Diogo Pereira nº 56302

-----------------------------------------------------------------------------------------------------

É utilizada a API POSIX, pelo que deve ser apenas utilizado em sistemas operativos UNIX.

O Makefile fornecido (executado através do comando "make") compila os ficheiros com a flag -g, 
de forma facilitar o debug dos mesmos e -Wall para alertar sobre os possíveis erros.
No mesmo Makefile foi ainda incluída uma função clean (executada com o comando "make clean"), que
remove todo o conteúdo gerado ao compilar os ficheiros c.

Foi também utilizada a função valgrind e verificou-se que não existem memory leaks.
