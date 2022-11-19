#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_COMMAND 1024
#define MAX_SELECT 3

int split_command(char *command, char *seperator, char *argv[]);

int main(void)
{
	// A. 변수
    char command[MAX_COMMAND];
	char *argv[MAX_SELECT];
	int argc;
	// B. 변수
	int pframe_num;
	// C. 변수

    // A. 실행할 알고리즘 선택
    printf("A. Rege Replacement 알고리즘 시뮬레이터를 선택하시오. (최대 3개)\n");
    printf(" (1) Optimal  (2) FIFO  (3) LIFO  (4) LRU  (5) LFU  (6) SC  (7) ESC  (8) ALL\n");
        
	fgets(command, sizeof(command), stdin);
	command[strlen(command) - 1] = '\0';
    argc = split_command(command," ",argv);

	// A. 입력받은 알고리즘 체크 : 1~3개를 입력받아야 함
	if (!(1 <= argc && argc <= 3))
	{
		printf("Input Error: 최소 1개에서 3개의 알고리즘 시뮬레이터를 선택하시오.\n");
		return 0;
	}
    argv[argc] = NULL;
    

	// B. 페이지 프레임 개수 선택
    printf("B. 페이지 프레임의 개수를 입력하시오. (3~10)\n");
	scanf("%d",&pframe_num);

	// B. 입력받은 페이지 프레임 개수 체크 : 3~10
	if(!(3 <= pframe_num && pframe_num <= 10))
	{
		printf("Input Error: 페이지 프레임의 개수 범위는 3~10 입니다.\n");
		return 0;
	}

	
	// C. 데이터 입력 방식 선택
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
