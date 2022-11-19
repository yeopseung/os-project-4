#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// 매크로
#define MAX_COMMAND 1024	// 명령어 최대 크기
#define MAX_SELECT 3		// 선택가능한 알고리즘 최대 크기
#define FNAME_R "samplestream.txt" // 사용자 생성 파일명

// 구조체
typedef struct Page
{
	int num;
}Page;

// 함수원형
int split_command(char *command, char *seperator, char *argv[]);
void create_random_pstream();
void create_fopen_pstream();
void print_pstream();
void optimal();


// 전역변수
Page pstream[100000];		// 페이지 스트림
int pstream_size = 500;		// 페이지 스트림 크기 (min: 500)
int pframe_num;				// 페이지 프레임 개수
int input_method;			// 데이터 입력 방식
bool isOptimal, isFIFO, isLIFO, isLRU, isLFU, isSC, isESC, isALL;	// 사용자가 선택할 알고리즘




int main(void)
{
	char command[MAX_COMMAND];	
	char *argv[MAX_SELECT];	
	int argc;	

	// A. 실행할 알고리즘 선택
    printf("\nA. Rege Replacement 알고리즘 시뮬레이터를 선택하시오. (최대 3개)\n");
    printf(" (1) Optimal  (2) FIFO  (3) LIFO  (4) LRU  (5) LFU  (6) SC  (7) ESC  (8) ALL\n");
	printf(">> ");

	fgets(command, sizeof(command), stdin);
	command[strlen(command) - 1] = '\0';
	argc = split_command(command," ",argv);
	

	// A. 입력받은 알고리즘 체크 : 1~3개를 입력받아야 함
	if (1 <= argc && argc <= 3)
	{
		for(int i=0; i<argc; i++)
		{
			switch (atoi(argv[i]))
			{
			case 1:
				isOptimal = true;
				break;
			case 2:
				isFIFO = true;
				break;
			case 3:
				isLIFO = true;
				break;
			case 4:
				isLRU = true;
				break;
			case 5:
				isLFU = true;
				break;
			case 6:
				isSC = true;
				break;
			case 7:
				isESC = true;
				break;
			case 8:
				isALL = true;
				break;
			
			default:
				fprintf(stderr,"Input Error: 선택 가능한 알고리즘 시뮬레이터 범위는 1~8 입니다.\n");
				exit(1);
				break;
			}
		}
	}
	else
	{
		fprintf(stderr,"Input Error: 최소 1개에서 3개의 알고리즘 시뮬레이터를 선택하시오.\n");
		exit(1);
	}
    

	// B. 페이지 프레임 개수 선택
    printf("\nB. 페이지 프레임의 개수를 입력하시오. (3~10)\n");
	printf(">> ");
	scanf(" %d",&pframe_num);

	// B. 입력받은 페이지 프레임 개수 체크 : 3~10
	if(!(3 <= pframe_num && pframe_num <= 10))
	{
		fprintf(stderr,"Input Error: 페이지 프레임의 개수 범위는 3~10 입니다.\n");
		exit(1);
	}
	

	// C. 데이터 입력 방식 선택
    printf("\nC. 데이터 입력 방식을 선택하시오. (1,2)\n");
    printf(" (1) 랜덤하게 생성  (2) 사용자 생성 파일 오픈\n");
	printf(">> ");
	scanf(" %d",&input_method);

	// C. 입력받은 데이터 입력 방식 체크
	switch (input_method)
	{
	case 1:
		create_random_pstream();
		//단 ESC 의 경우 다르게 입력 받아야함.
		break;
	case 2:
		create_fopen_pstream();
		//단 ESC 의 경우 다르게 입력 받아야함.
		break;

	default:
		fprintf(stderr,"Input Error: 데이터 입력 방식 선택은 1 또는 2 입니다.\n");
		exit(1);
		break;
	}

	print_pstream();

	// 페이지 교체 알고리즘 실행
	// 다른 알고리즘과 비교를 위해 Optimal 알고리즘은 기본적으로 실행
	optimal();
	
    



	// D. 종료
    printf("D. 종료\n");

    
}

// 명령어 스트링을 공백으로 분할
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


// 랜덤하게 페이지 스트림 생성 (기본적으로 최소 500개의 페이지 스트링)
void create_random_pstream()
{	
	//난수 초기화
	srand(time(NULL));	

	//최소 500개의 페이지 스트링
	for(int i=0; i<500; i++)
	{	
		// 1~30 범위의 페이지 스트링
		int random = (rand() % 30) + 1;
		pstream[i].num = random;
	}
}

// 사용자가 생성한 파일을 열어 페이지 스트림 생성
void create_fopen_pstream()
{	
	FILE *fp;
	int size;

	if(access(FNAME_R, F_OK) != -1)
	{
		// 사용자 생성 파일 존재
		// -> 파일을 열어 스트림 read

		int cnt = 0;

		if((fp = fopen(FNAME_R,"r")) == NULL)
		{
			fprintf(stderr, "fopen error for %s\n",FNAME_R);
			exit(1);
		}

		fseek(fp,0,SEEK_END);
		size = ftell(fp);
		fseek(fp,0,SEEK_SET);

		while(!feof(fp))
		{
			fscanf(fp,"%d",&pstream[cnt].num);
			cnt++;
		}

		// 페이지 스트림 개수 업데이트 (default: 500)
		pstream_size = cnt-1;	

		fclose(fp);
	}
	else
	{
		// 사용자 생성 파일이 존재하지 않을 경우 
		// -> 랜덤한 페이지 스트림 생성, 파일에 write
		
		create_random_pstream();

		if((fp = fopen(FNAME_R,"w+")) == NULL)
		{
			fprintf(stderr, "fopen error for %s\n",FNAME_R);
			exit(1);
		}

		for(int i=0; i< (sizeof(pstream) / sizeof(Page)); i++)
		{	
			if(pstream[i].num == 0)
			{
				break;
			}
			fprintf(fp,"%d ",pstream[i].num);
		}

		fclose(fp);
	}
}

void print_pstream()
{	
	printf("\n>> Page Stream <<\n");
	for(int i=0; i<pstream_size; i++)
	{	
		printf("%d ",pstream[i].num);
		if((i%50) == 0 && i != 0)
		{
			printf("\n");
		}
	}
	printf("\n\n");
}

void optimal()
{

}