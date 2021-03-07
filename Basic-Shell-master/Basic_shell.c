#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

//functie pentru eliberarea ecranului/ CTRL + L
//#define clear() printf("\033[H\033[J")


//culori pt consola
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"


//pentru com1 > com2 , com1 < com2

#define INPUT 0
#define OUTPUT 1



//elimina spatiile libere

void removeWhiteSpace(char* buf){
	if(buf[strlen(buf)-1]==' ' || buf[strlen(buf)-1]=='\n')
	buf[strlen(buf)-1]='\0';
	if(buf[0]==' ' || buf[0]=='\n') memmove(buf, buf+1, strlen(buf));
}


//imparte buf folosind delimitatorul c si returneaza un vector de string uri in param si dimensiunea sa
void tokenize_buffer(char** param,int *nr,char *buf,const char *c){
	char *token;
	token=strtok(buf,c);
	int pc=-1;
	while(token){
		param[++pc]=malloc(sizeof(token)+1);
		strcpy(param[pc],token);
		removeWhiteSpace(param[pc]);
		token=strtok(NULL,c);
	}
	param[++pc]=NULL;
	*nr=pc;
}


//comenzi simple
void executeBasic(char** argv){
	if(fork()>0){
		//parinte
		wait(NULL);
	}
	else{
		//copil
		execvp(argv[0],argv);//executa comanda 
		//eroare
		perror(ANSI_COLOR_RED   "invalid input"   ANSI_COLOR_RESET "\n");
		exit(1);
	}
}

//comzi de tip pipe 
void executePiped(char** buf,int nr){//maxim 10 pipes
	if(nr>10) return;
	
	int fd[10][2],i,pc;
	char *argv[100];

	for(i=0;i<nr;i++){
		tokenize_buffer(argv,&pc,buf[i]," ");
		//argv va contine comanda si parametrii
		if(i!=nr-1){
			if(pipe(fd[i])<0){
				perror("pipe creating was not successfull\n");
				return;
			}
		}
		//fd[i][0]- cap citire
		//fd[i][1]- cap scriere
		if(fork()==0){//copil
			if(i!=nr-1){
			//scrie
				dup2(fd[i][1],1);//se face o copie a file descriptorului
				close(fd[i][0]);
				close(fd[i][1]);
			}

			if(i!=0){
			//citeste
				dup2(fd[i-1][0],0); //se face o copie a file descriptoeului
				close(fd[i-1][1]);
				close(fd[i-1][0]);
			}	
			//se executa comanda de la pasul i
			execvp(argv[0],argv);
			perror("invalid input ");
			exit(1);//eroare
		}
		//parinte
		if(i!=0){//inchidem fisierul anterior
			close(fd[i-1][0]);
			close(fd[i-1][1]);
		}
		wait(NULL);
	}
}

//comenzi asincrone
void executeAsync(char** buf,int nr){
	int i,pc;
	char *argv[100];
	for(i=0;i<nr;i++){
		tokenize_buffer(argv,&pc,buf[i]," ");
		if(fork()==0){
			execvp(argv[0],argv);
			perror("invalid input");
			exit(1);
		}
	}
	for(i=0;i<nr;i++){
		wait(NULL);
	}

}

//redirectionarea fisierelor
void executeRedirect(char** buf,int nr,int mode){
	int pc,fd;
	char *argv[100];
	removeWhiteSpace(buf[1]);
	tokenize_buffer(argv,&pc,buf[0]," ");
	if(fork()==0){

		switch(mode){
		case INPUT:  fd=open(buf[1],O_RDONLY|O_CREAT,0000744); break;//se deschide fisierul pt citire
		case OUTPUT: fd=open(buf[1],O_WRONLY|O_CREAT,0000744); break;//se deschide fisierul pt scriere
		
		default: return;
		}

		if(fd<0){
			perror("cannot open file\n");
			return;
		}

		switch(mode){
		case INPUT:  		dup2(fd,0); break;
		case OUTPUT: 		dup2(fd,1); break;
		
		default: return;
		}
		execvp(argv[0],argv);
		perror("invalid input ");
		exit(1);
	}
	wait(NULL);
}

//help
void showHelp(){
	printf(ANSI_COLOR_GREEN   "----------Help--------"   ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "Nu toate comenzile interne sunt acceptate"  ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "Suporta comenzile interne: cd, pwd, echo, exit "   ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "Comenzile pot fi de tip pipe (maxim 10)"   ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "Ex. ls -a | wc -c este permisa"   ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "Comenzile asincron sunt permise "   ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "Ex. ls -a & cat file este permisa"   ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "Redirectionare de fisiere (de iesire): ex. ls > fileOutput "   ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "Redirectionare de fisiere (de intrare): ex. wc -c < fileInput "   ANSI_COLOR_RESET "\n");
}

int history()
{
	//returneaza informatii legate de lista istoricului(lungime,dimensiune)
	HISTORY_STATE *myhist = history_get_history_state();

	//intoarce istoricul comenzilor
	HIST_ENTRY **mylist = history_list();

	for (int i = 0; i < myhist->length; i++)
	{//afisare continut
		printf("%8s %s\n", mylist[i]->line, mylist[i]->timestamp);
		
	}
	putchar('\n');

	free(myhist); 
	
}
int main(char** argv,int argc)
{
	//clear();
	char buf[500],*buffer[100],buf2[500],buf3[500], *params1[100],*params2[100],*token,cwd[1024];
	int nr=0;
	
	printf(ANSI_COLOR_GREEN   "*****************************************************************"   ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN   "************************** SHELL LUIZA && BIANCA ***************************"   ANSI_COLOR_RESET "\n");

	while(1){
		printf(ANSI_COLOR_BLUE   "Enter command(or 'exit' to exit):"   ANSI_COLOR_RESET "\n");

		//directorul curent
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		printf(ANSI_COLOR_GREEN "%s  " ANSI_COLOR_RESET, cwd);
		else 	perror("getcwd failed\n");

		//citire date
		fgets(buf, 500, stdin);
		add_history(buf);

		//verificam daca trebuie executata o comanda simpla sau una compusa

		if(strchr(buf,'|')){//comanda pipe
			tokenize_buffer(buffer,&nr,buf,"|");
			executePiped(buffer,nr);
		}
		else if(strchr(buf,'&')){//comanda asincron
			tokenize_buffer(buffer,&nr,buf,"&");
			executeAsync(buffer,nr);
		}
		
		else if(strchr(buf,'>')){//redirectioneaza output-ul spre fisier
			tokenize_buffer(buffer,&nr,buf,">");
			if(nr==2)executeRedirect(buffer,nr, OUTPUT);
			else printf("Incorrect output redirection!(has to to be in this form: command > file)");
		}
		else if(strchr(buf,'<')){//redirectioneaza fisierul catre input
			tokenize_buffer(buffer,&nr,buf,"<");
			if(nr==2)executeRedirect(buffer,nr, INPUT);
			else printf("Incorrect input redirection!(has to to be in this form: command < file)");
		}
		else{//comanzi simple si inclusiv cele interne
			tokenize_buffer(params1,&nr,buf," ");
			if(strstr(params1[0],"cd")){//cd 
				chdir(params1[1]);
			}
			else if(strstr(params1[0],"help")){//help 
				showHelp();
			}
			else if(strstr(params1[0],"exit")){//exit 
				exit(0);
			}
			else if(strstr(params1[0],"history")){//history
				history();
			}
			//comanda simpla
			else executeBasic(params1);
		}
	}

	return 0;
}
