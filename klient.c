#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>


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
} msg , msgr;


int meniu2(int logpasid){
	//logowanko
	
	printf("\e[1;1H\e[2J");
	printf("logowanko\n\n");
	int czyokid = msgget(getpid() , 0644 | IPC_CREAT);
	logpas.typ = 3;
	logpas.pid = getpid();
	char * cos;
	printf("dawaj login\n");

	scanf("%s" , logpas.log);
	cos = getpass("dawaj haslo\n");
	strcpy(logpas.pass , cos);
	msgsnd(logpasid , &logpas , sizeof(logpas) - sizeof(long) , 0);
	msgrcv(czyokid , &czyok , sizeof(czyok) - sizeof(long) , 3 , 0);
	msgctl(czyokid , IPC_RMID , NULL);
	if(czyok.ok){
		strcpy(msg.kto , logpas.log);
		msg.ktokey = czyok.ok;
		return 21;
	}
	return 1;
}

int meniu3(int logpasid){
	//rejestrowanko
	
	printf("\e[1;1H\e[2J");
	printf("rejestrowanko\n\n");
	logpas.typ = 4;
	logpas.pid = getpid();
	char * cos;
	printf("dawaj login\n");
	scanf("%s" , logpas.log);
	cos = getpass("dawaj pass\n");
	strcpy(logpas.pass , cos);
	msgsnd(logpasid , &logpas , sizeof(logpas) - sizeof(long) , 0);
	return 1;
}

int meniu1(){
    printf("\e[1;1H\e[2J");
    printf("(l) - zaloguj\n");
    printf("(r) - zarejestruj\n");
    printf("(w) - wyjscie\n");
    char ch;
    scanf("%c", &ch);
    if (ch == 'l') return 2;
    if (ch == 'r') return 3;
    if (ch == 'w') return 0;
    return 1;
}

int meniu21();

int odbierzwazne(){
    int odbid = msgget(czyok.ok , 0644 | IPC_CREAT);
    while(1){
        if(msgrcv(odbid, &msgr, sizeof(msgr) - sizeof(long), 10 , IPC_NOWAIT) != -1){
            printf("\e[1;1H\e[2J");
            printf("%s\n\n", msgr.temat);
            printf("%s\n", msgr.co);
            getchar();
            getchar();
            meniu21(getpid());
        }

        sleep(2);
    }
}

int meniu21(int dzieckodozamordowania){
    if(dzieckodozamordowania != 0) kill(dzieckodozamordowania , SIGINT);
    dzieckodozamordowania = fork();
    if(dzieckodozamordowania != 0) {
        printf("\e[1;1H\e[2J");
        printf("%d\n" , getppid());
        printf("zalogowano: %s %d\n\n", logpas.log, msg.ktokey);
        printf("(d) - dodaj temat\n");
        printf("(s) - wyslij wiadomosc\n");
        printf("(t) - subskrybuj temat\n");
        printf("(b) - blokuj uzytownika\n");
        printf("(o) - odbierz wiadomosci\n");
        printf("(w) - wroc\n");
        char ch;
        scanf("%c", &ch);
        scanf("%c", &ch);
        if (ch == 'd') return 211;
        if (ch == 's') return 212;
        if (ch == 't') return 213;
        if (ch == 'b') return 214;
        if (ch == 'o') return 215;
        if (ch == 'w') {
            kill(dzieckodozamordowania , SIGINT);
            msgctl(msg.ktokey, IPC_RMID, NULL);
            return 1;
        }
        return 21;
    }
    else{
        odbierzwazne();
    }
}

int meniu211(int msgid){
	printf("\e[1;1H\e[2J");
	printf("jaki chcesz dodac temat?\n");
	scanf("%s" , msg.temat);
	msg.typ = 3;
	msgsnd(msgid , &msg , sizeof(msg) - sizeof(long) , 0);
	return 21;
}

int meniu212(int msgid){
	printf("\e[1;1H\e[2J");
	msg.typ = 6;
	printf("jaki temat?\n");
	scanf("%s" , msg.temat);
	printf("jaki priorytet?(0-20)\n");
	scanf("%d" , &msg.priorytet);
	printf("co?\n");
	scanf("%s" , msg.co);
	msgsnd(msgid , &msg , sizeof(msg) - sizeof(long) , 0);
	return 21;
}

int meniu213(int msgid){
    printf("\e[1;1H\e[2J");
    msg.typ = 4;
    printf("jaki temat subowac?\n");
    scanf("%s" , msg.temat);
    msgsnd(msgid , &msg , sizeof(msg) - sizeof(long) , 0);
    return 21;
}

int meniu214(int msgid){
    printf("\e[1;1H\e[2J");
    msg.typ = 5;
    printf("kogo blokowac?\n");
    scanf("%s" , msg.temat);
    msgsnd(msgid , &msg , sizeof(msg) - sizeof(long) , 0);
    return 21;
}

int meniu215(){
    printf("\e[1;1H\e[2J");
    int odbid = msgget(czyok.ok , 0644 | IPC_CREAT);
    if(odbid != -1) {
        for(int i = 1; i < 21; i++) {
            while (msgrcv(odbid, &msgr, sizeof(msgr) - sizeof(long), 10 + i, IPC_NOWAIT) != -1) {
                printf("\e[1;1H\e[2J");
                printf("%s\n\n", msgr.temat);
                printf("%s\n", msgr.co);
                getchar();
            }
        }
    }
    return 21;
}

int main(int argc , char* argv[]){
    int logpasid = msgget(33710 , 0644);
	int msgid = msgget(33170 , 0644);
	int stan = 1;
	while(stan){
		switch(stan) {
			case 0:
				stan = 0;
				break;
			case 1:
				stan = meniu1();
				break;
			case 2:
				stan = meniu2(logpasid);
				break;
			case 3:
				stan = meniu3(logpasid);
				break;
			case 21:
				stan = meniu21(0);
				break;
			case 211:
				stan = meniu211(msgid);
				break;
            case 212:
                stan = meniu212(msgid);
                break;
            case 213:
                stan = meniu213(msgid);
                break;
            case 214:
                stan = meniu214(msgid);
                break;
		    case 215:
		        stan = meniu215();
		        break;
			default:
				break;
		}
	}
	
    return 0;
}










