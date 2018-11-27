#include <stdio.h>
#include <signal.h>

int flag = 0;

void child_handler (int signo)
{
	flag = 1;

}

void mywait()
{
	struct sigaction act;
	sigfillset(&act.sa_mask);
	act.sa_handler = child_handler;
	sigaction(SIGCHLD, &act, NULL);

	while(flag != 1)
	{
		pause();
	}
	return;
}

int main()
{

	pid_t pid = fork();
	
	if (pid == 0) {
		printf("hey im a child!\n");
		sleep(5);
	
	} 
	else
	{
		mywait();
		printf("done waiting\n");
	}
	
	return 0;
}
