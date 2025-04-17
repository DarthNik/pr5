#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

void handler(int sig, siginfo_t *info, void *context){
        printf("Игра начинается\n");
}

void player1(pid_t pid, int i, int n, sigset_t set, sigset_t oldset);
void player2(pid_t pid, int i, int n, sigset_t set);

int main(int argc, char* argv[]){
    pid_t pid = fork();
    if (pid < 0){
	perror("fork error");
	exit(1);
    }

    int n = atoi(argv[1]);

    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGRTMIN);
    sigaddset(&set, SIGRTMIN+1);
    sigprocmask(SIG_BLOCK, &set, &oldset);
    
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    act.sa_mask = set;
    if (sigaction(SIGRTMIN, &act, 0) == -1)
	perror("can't catch");

    srand(time(NULL));

    for (int i = 1; i <= 10; i++){
        if (i%2 == 1){
	    if (pid > 0)
	        player2(pid, i, n, set);

	    else
	        player1(getppid(), i, n, set, oldset);
	}
	
	else {
	    if (pid > 0)
	        player1(pid, i, n, set, oldset);

	    else
	        player2(getppid(), i, n, set);
	}
    }

    if (pid > 0)
	wait(NULL);
 
    return 0;
}


void player1(pid_t pid, int i, int n, sigset_t set, sigset_t oldset){
    sigsuspend(&oldset);
    
    int guess = 0;
    while (1){
        guess++;
        union sigval value;
        value.sival_int = guess;
        sigqueue(pid, SIGRTMIN+1, value);
        siginfo_t si;
        int sig = sigwaitinfo(&set, &si);
        if (sig == SIGUSR1)
            break;
        else if (sig == SIGUSR2)
            continue;
        }

}

void player2(pid_t pid, int i, int n, sigset_t set){
    int target = rand() % n + 1;
    int attempt = 0;
    printf("Раунд %d:\n", i);
    printf("Процесс %d загадывает число от 1 до %d\n", getpid(), n);
    if (kill(pid, SIGRTMIN) < 0){
        perror("kill");
        exit(1);
    }

    while (1){
        siginfo_t si;
	int sig = sigwaitinfo(&set, &si);
	if (sig == SIGRTMIN+1){
	    int guess = si.si_value.sival_int;
	    attempt++;
	    printf("\tПопытка %d: %d\n", attempt,  guess);
	    if (guess == target){
	        kill(pid, SIGUSR1);
		printf("Раунд %d завершён\nКоличество попыток: %d\n", i, attempt);
		break;
	    }
	    else
		kill(pid, SIGUSR2);
         }
    }
}
