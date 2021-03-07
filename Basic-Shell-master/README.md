# Basic-Shell

* _void removeWhiteSpace(char* buf)_ – elimina spatiile.
* _void tokenize_buffer(char** param,int *nr,char *buf,const char *c)_ – imparte buf folosind delimitatorul c si returneaza un vector de string-uri in param si dimensiunea sa in nr.
* _void executeBasic(char** argv)_ – executa comenzile simple, folosind functia de sistem execvp().
* _void executePiped(char** buf,int nr)_ – executa comenzile de tip pipe ( maxim 10 comenzi ), folosind functia de sistem pipe() pentru a crea un cap de citire si scriere, dup2() pentru a face o copie a file descriptorului si execvp(argv[0],argv) pentru a executa comanda din argv[0] folosind parametrii argv.
* _void executeAsync(char** buf,int nr)_ – executa comenzi asincrone
* _void executeRedirect(char** buf,int nr,int mode)_ – redirectionare de fisiere; redirectionare output spre fisier si redirectioneaza fisierul spre input

***Pentru rulare:  gcc ProiectShell.c –o Proiect -lreadline***
