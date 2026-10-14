/*
 * 1_pipe_basic.c — 기본 예제(단방향 파이프)를 왕복 통신으로 변형
 * 실행: ./1_pipe "hello world"
 * 부모 -> 자식: 문자열 / 자식 -> 부모: 대문자로 바꾼 문자열
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int parent_to_child[2], child_to_parent[2];
    char buf[1024];
    ssize_t n;
    int status;

    if (argc != 2 || strlen(argv[1]) >= sizeof(buf)) {
        fprintf(stderr, "사용법: %s \"문자열(1023바이트 이하)\"\n", argv[0]);
        return 1;
    }
    /* 파이프는 단방향이므로 왕복 통신에 두 개가 필요하다. */
    if (pipe(parent_to_child) == -1) { perror("pipe 1"); return 1; }
    if (pipe(child_to_parent) == -1) {
        perror("pipe 2");
        close(parent_to_child[0]); close(parent_to_child[1]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(parent_to_child[0]); close(parent_to_child[1]);
        close(child_to_parent[0]); close(child_to_parent[1]);
        return 1;
    }
    if (pid == 0) {
        /* 자식: 부모 메시지를 읽고 대문자로 바꿔 다시 전송한다. */
        close(parent_to_child[1]); /* 부모 -> 자식 쓰기 끝은 사용 안 함 */
        close(child_to_parent[0]); /* 자식 -> 부모 읽기 끝은 사용 안 함 */

        size_t used = 0;
        while (used < sizeof(buf)-1 &&
               (n = read(parent_to_child[0], buf + used, sizeof(buf)-1-used)) > 0)
            used += (size_t)n;
        if (n < 0) { perror("child read"); _exit(1); }
        for (size_t i = 0; i < used; i++)
            buf[i] = (char)toupper((unsigned char)buf[i]);
        close(parent_to_child[0]);

        if (write(child_to_parent[1], buf, used) != (ssize_t)used) {
            perror("child write"); _exit(1);
        }
        close(child_to_parent[1]); /* 쓰기를 끝내 부모에게 EOF 전달 */
        _exit(0);
    }

    /* 부모: 자식에게 쓰고, 자식의 응답을 읽는다. */
    close(parent_to_child[0]);
    close(child_to_parent[1]);
    printf("[부모] 보낸 내용: %s\n", argv[1]);
    size_t length = strlen(argv[1]);
    if (write(parent_to_child[1], argv[1], length) != (ssize_t)length) {
        perror("parent write");
        close(parent_to_child[1]); close(child_to_parent[0]);
        waitpid(pid, &status, 0);
        return 1;
    }
    close(parent_to_child[1]); /* 자식이 EOF를 볼 수 있도록 닫는다. */

    size_t used = 0;
    while (used < sizeof(buf)-1 &&
           (n = read(child_to_parent[0], buf + used, sizeof(buf)-1-used)) > 0)
        used += (size_t)n;
    if (n < 0) { perror("parent read"); close(child_to_parent[0]); waitpid(pid, &status, 0); return 1; }
    buf[used] = '\0';
    printf("[부모] 자식의 응답: %s\n", buf);
    close(child_to_parent[0]);
    if (waitpid(pid, &status, 0) == -1) { perror("waitpid"); return 1; }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}
