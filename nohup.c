#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/param.h>

void closefd(void) {
	for(int i=0; i<sysconf(_SC_OPEN_MAX); i++) {
		close(i);
	}
}

int dup2file(char* filename) {
	int fd = open(filename, O_RDWR|O_CREAT, 0666);
	if (fd == -1) {
		perror("open error");
		return -1;
	}
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) {
		close(fd);
	}
	return 0;
}

int mydaemon(char* logfile) {
	//1.首次fork;
	//防止当前进程是首进程，首进程调用setsid会失败；故需要fork确保setsid成功
	pid_t pid = fork();
	if(pid == -1) {
		perror("fork error");
		return -1;
	}
	if(pid > 0) {
		_exit(EXIT_SUCCESS);
	}
	//2.创建新会话
	//创建新会话后，当前进程就跟原会话和控制终端断开连接，不再受其影响
	if(setsid() == -1) {
		perror("setsid error");
		return -1;
	}
	//3.创建子进程,
	//因为setsid后，进程成了新会话和新进程组的首进程，即领导进程，还是能够打开控制终端；为了防止进程重新获取控制终端，再次fork
	pid = fork();
	if(pid == -1) {
		perror("fork error");
		return -1;
	}
	if(pid > 0) {
		_exit(EXIT_SUCCESS);
	}
	signal(SIGCHLD, SIG_IGN);
	//4.设置掩码
	umask(0);
	//5.切换工作目录至根目录
	//chdir("/");
	//6.重定向标准输入输出
	closefd();
	dup2file(logfile);	
}

int main(int argc, char* argv[]) {
	char logfile[MAXPATHLEN] = "";
	getcwd(logfile, MAXPATHLEN);
	strncat(logfile, "/nohup.log", 10);

	if (argc < 2) {
		printf("Usage: %s [command]\n", argv[0]);
		return -1;
	}

	mydaemon(buf);
	if (execlp(argv[1], argv[1], NULL) == -1) {
		perror("exec error");
	}
	return 0;
}
