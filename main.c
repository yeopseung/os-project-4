#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_COMMAND 1024
#define MAX_SELECT 3

int split_command(char *command, char *seperator, char *argv[]);

int main(void)
{
    char command[MAX_COMMAND];
	char *argv[MAX_SELECT];
	int argc;


    // 실행할 알고리즘 선택
    printf("A. Rege Replacement 알고리즘 시뮬레이터를 선택하시오. (최대 3개)\n");
    printf(" (1) Optimal  (2) FIFO  (3) LIFO  (4) LRU  (5) LFU  (6) SC  (7) ESC  (8) ALL\n");
        
	fgets(command, sizeof(command), stdin);
	command[strlen(command) - 1] = '\0';
    argc = split_command(command," ",argv);
        

    printf("B. 페이지 프레임의 개수를 입력하시오. (3~10)\n");

    printf("C. 데이터 입력 방식을 선택하시오. (1,2)\n");
    printf(" (1) 랜덤하게 생성  (2) 사용자 생성 파일 오픈\n");

    printf("D. 종료\n");

    
}

int split_command(char *command, char *seperator, char *argv[])
{
	int argc = 0;
	char *ptr = NULL;

	ptr = strtok(command, seperator);
	while (ptr != NULL)
	{
		argv[argc++] = ptr;
		ptr = strtok(NULL, " ");
	}

	return argc;
}
