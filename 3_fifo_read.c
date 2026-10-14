/*
 * 3_fifo_read.c — FIFO 수신 예제 변형 [먼저 실행]
 * 실행: ./3_fifo_read  (다른 터미널에서 ./4_fifo_write "hello fifo")
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_PATH "./message_fifo"  /* 프로젝트 폴더에서 생성하고 삭제 */

int main(void)
{
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo"); return 1;
    }
    printf("[수신] 보내는 프로그램을 기다립니다...\n");
    fflush(stdout);

    /* 상대가 O_WRONLY로 열기 전까지 open이 대기한다. */
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open read"); unlink(FIFO_PATH); return 1;
    }

    printf("[수신] 받은 메시지: ");
    fflush(stdout);
    char buf[128];
    ssize_t n;
    /* 교수님 원본의 한 번 읽기를 반복 읽기로 바꿔 긴 메시지도 처리한다. */
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        fwrite(buf, 1, (size_t)n, stdout);
    putchar('\n');
    if (n < 0) perror("read");
    close(fd);
    if (unlink(FIFO_PATH) == -1) { perror("unlink"); return 1; }
    return n < 0 ? 1 : 0;
}
