/*
 * 4_fifo_write.c — FIFO 송신 예제 변형 [나중에 실행]
 * 실행: ./4_fifo_write "hello fifo"
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_PATH "./message_fifo"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "사용법: %s \"메시지\"\n", argv[0]);
        return 1;
    }
    /* 받는 쪽이 FIFO 파일을 먼저 만들고 열어야 한다. */
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open write (수신 프로그램을 먼저 실행했는지 확인)");
        return 1;
    }
    size_t len = strlen(argv[1]);
    if (write(fd, argv[1], len) != (ssize_t)len) {
        perror("write"); close(fd); return 1;
    }
    printf("[송신] 보낸 메시지: %s\n", argv[1]);
    close(fd);
    return 0;
}
