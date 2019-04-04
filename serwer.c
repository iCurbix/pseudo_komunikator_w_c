#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <string.h>
#include <crypt.h>
#include <unistd.h>
#include <time.h>

int main(int argc , char* argv[]){
	srand(time(NULL));
    int logpasid = msgget(33710 , 0644 | IPC_CREAT);
	int msgid = msgget(33170 , 0644 | IPC_CREAT);
	struct logpasbuf{
        long typ;
        int pid;
		char log[128];
		char pass[128];
    } logpas;
	
	struct czyokbuf{
        long typ;
        int ok;
    } czyok;
	
	struct wiadomosc{
		long typ;
		char temat[128];
		char co[512];
		char kto[128];
		int ktokey;
		int priorytet;
	} msg;

	struct stat st = {0};

	if (stat("plikersy", &st) == -1) {
		mkdir("plikersy", 0744);
	}
	if (stat("plikersy/suby", &st) == -1) {
		mkdir("plikersy/suby", 0744);
	}
	if (stat("plikersy/bany", &st) == -1) {
		mkdir("plikersy/bany", 0744);
	}

	char *passwd;
	if(fork()){
		while(1){
			if(msgrcv(logpasid , &logpas , sizeof(logpas) - sizeof(long) , 0 , IPC_NOWAIT) != -1){		
				if(logpas.typ == 3){
					
					//logowanko
					
					int passyid = open("plikersy/passy.lopa" , O_RDONLY);
					passwd = crypt(logpas.pass , "$1$qe");
					printf("log\n%s\n%s\n%s\n\n" , logpas.log , logpas.pass , passwd);
					czyok.ok = 0;
					int n;
					char cos[128];
					while(n = read(passyid , cos , 128)){
						if(strcmp(cos , logpas.log) == 0){
							read(passyid , cos , 128);
							if(strcmp(passwd , cos) == 0){
								read(passyid , &czyok.ok , sizeof(int));
								break;
							}
						}
						lseek(passyid , 128 , SEEK_CUR);
						lseek(passyid , sizeof(int) , SEEK_CUR);
					}
					czyok.typ = 3;
					msgsnd(msgget(logpas.pid , 0644) , &czyok , sizeof(czyok) - sizeof(long) , 0);
					close(passyid);
				}
				if(logpas.typ == 4){
					
					//rejestrowanko
					
					int passyid = open("plikersy/passy.lopa" , O_RDWR|O_CREAT , 0644);
					int n , ok = 1;
					char cos[128];
					while(n = read(passyid , cos , 128)){
						if(strcmp(cos , logpas.log) == 0){
							ok = 0;
							break;
						}
						lseek(passyid , 128 , SEEK_CUR);
						lseek(passyid , sizeof(int) , SEEK_CUR);
					}
					if(!ok){
						close(passyid);
						continue;
					}
					passwd = crypt(logpas.pass , "$1$qe");
					printf("rej\n%s\n%s\n%s\n\n" , logpas.log , logpas.pass , passwd);
					write(passyid , logpas.log , 128);
					write(passyid , passwd , 128);
					int ktok = rand();
					write(passyid , &ktok , sizeof(int));
					close(passyid);
				}	
			
			
			
			
			
			}	
		}
	}
	else{
		while(1){
			if(msgrcv(msgid , &msg , sizeof(msg) - sizeof(long) , 0 , IPC_NOWAIT) != -1) {
				if (msg.typ == 3) {
					int tematyid = open("plikersy/tematy.tmt", O_RDWR | O_CREAT, 0644);
					printf("dod\n%s\n", msg.temat);
					char cos[128];
					int ok = 1;
					while (read(tematyid, cos, 128)) {
						if (strcmp(cos, msg.temat) == 0) {
							ok = 0;
							break;
						}
					}
					if (!ok){
						close(tematyid);
						continue;
					}
					write(tematyid, msg.temat, 128);
					close(tematyid);
				}
				if (msg.typ == 4) {
					msg.kto[strlen(msg.kto) + 1] = 0;
					msg.kto[strlen(msg.kto)] = 's';
					char src[128];
					strcpy(src, "plikersy/suby/");
					strcat(src, msg.kto);
					printf("subowanie\n%s\n", src);
					int suby = open(src, O_RDWR | O_CREAT, 0644);
					char cos[128];
					int ok = 1;
					while (read(suby, cos, 128)) {
						if (strcmp(cos, msg.temat) == 0) {
							ok = 0;
							break;
						}
					}
					if (!ok){
						close(suby);
						continue;
					}
					write(suby, msg.temat, 128);
					close(suby);
				}
				if (msg.typ == 5) {
					msg.kto[strlen(msg.kto) + 1] = 0;
					msg.kto[strlen(msg.kto)] = 'b';
					char src[128];
					strcpy(src, "plikersy/bany/");
					strcat(src, msg.kto);
					printf("banowanie\n%s\n", src);
					int bany = open(src, O_RDWR | O_CREAT, 0644);
					char cos[128];
					int ok = 1;
					while (read(bany, cos, 128)) {
						if (strcmp(cos, msg.temat) == 0) {
							ok = 0;
							break;
						}
					}
					if (!ok){
						close(bany);
						continue;
					}
					write(bany, msg.temat, 128);
					close(bany);
				}
				if (msg.typ == 6){
					int passyid = open("plikersy/passy.lopa" , O_RDONLY);
					char nazwa[128];
					char src[128];
					strcpy(src, "plikersy/bany/");
					int klucz;

					//branie wszytkich potencjalnych odbiorcow
					while(read(passyid , nazwa , 128)){
						lseek(passyid , 128 , SEEK_CUR);
						read(passyid , &klucz , sizeof(int));

						//sprawdzanie czy baowany

						nazwa[strlen(nazwa) + 1] = 0;
						nazwa[strlen(nazwa)] = 'b';
						strcat(src , nazwa);
						int ok = 1;
						char cos[128];
						int bany = open(src, O_RDONLY);
						if(bany != -1) {
							printf("%s\n%d\n" , src , bany);
							while (read(bany, cos, 128)) {
								if (strcmp(cos, msg.kto) == 0) {
									ok = 0;
									break;
								}
							}
							close(bany);
							if (!ok) continue;
						}
						//sprawdzanie czy subskrybowane

						nazwa[strlen(nazwa) - 1] = 's';
						strcpy(src, "plikersy/suby/");
						strcat(src , nazwa);
						int suby = open(src, O_RDONLY);
						if(suby == -1) continue;
						ok = 0;
						while (read(suby, cos, 128)) {
							if (strcmp(cos, msg.temat) == 0) {
								ok = 1;
								break;
							}
						}
						close(suby);
						if(!ok) continue;

						//wyslanie
						printf("wys\n%s\n%s\n\n" , nazwa , msg.temat);
						int ktosid = msgget(klucz , 0644 | IPC_CREAT);
						msg.typ = (msg.priorytet % 20) + 10;
						msgsnd(ktosid , &msg , sizeof(msg) - sizeof(long) , 0);
					}
					close(passyid);
				}
			}
		}
	}

    return 0;
}