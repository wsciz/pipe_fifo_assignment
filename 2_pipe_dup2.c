/*
 * 2_pipe_dup2.c — ls | wc -l 고정 예제를 두 명령어 실행으로 변형
 * 실행: ./2_pipeline ls -l : wc -l
 * ':' 앞 명령의 표준출력과 뒤 명령의 표준입력을 dup2로 연결한다.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int sep = -1, fd[2], first_status, second_status;
    pid_t first_pid, second_pid;

    /* 명령행에서 ':'를 찾아 두 명령으로 분리한다. */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ":") == 0) { sep = i; break; }
    }
    if (sep < 2 || sep >= argc - 1) {
        fprintf(stderr, "사용법: %s 명령1 [인자] : 명령2 [인자]\n", argv[0]);
        return 1;
    }
    argv[sep] = NULL;
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    /* 첫 번째 자식: 첫 명령의 표준출력(1)을 파이프에 연결한다. */
    first_pid = fork();
    if (first_pid == -1) {
        perror("fork first"); close(fd[0]); close(fd[1]); return 1;
    }
    if (first_pid == 0) {
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) == -1) { perror("dup2 stdout"); _exit(1); }
        close(fd[1]);
        execvp(argv[1], &argv[1]);
        perror("execvp first");
        _exit(127);
    }

    /* 두 번째 자식: 둘째 명령의 표준입력(0)을 파이프에 연결한다. */
    second_pid = fork();
    if (second_pid == -1) {
        perror("fork second");
        close(fd[0]); close(fd[1]); waitpid(first_pid, NULL, 0);
        return 1;
    }
    if (second_pid == 0) {
        close(fd[1]);
        if (dup2(fd[0], STDIN_FILENO) == -1) { perror("dup2 stdin"); _exit(1); }
        close(fd[0]);
        execvp(argv[sep + 1], &argv[sep + 1]);
        perror("execvp second");
        _exit(127);
    }

    /* 부모가 두 끝 모두 닫지 않으면 wc가 EOF를 못 받아 멈출 수 있다. */
    close(fd[0]);
    close(fd[1]);
    if (waitpid(first_pid, &first_status, 0) == -1) { perror("wait first"); return 1; }
    if (waitpid(second_pid, &second_status, 0) == -1) { perror("wait second"); return 1; }
    return WIFEXITED(first_status) && WEXITSTATUS(first_status) == 0 &&
           WIFEXITED(second_status) && WEXITSTATUS(second_status) == 0 ? 0 : 1;
}
