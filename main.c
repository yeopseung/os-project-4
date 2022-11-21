#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// 매크로
#define MAX_COMMAND 1024	// 명령어 최대 크기
#define MAX_SELECT 3		// 선택가능한 알고리즘 최대 크기
#define MAX_PSTREAM 100000	// 페이지 스트림 최대 크기
#define MIN_PSTREAM 500		// 페이지 스트림 최소 크기
#define FNAME_R "samplestream.txt" // 사용자 생성 파일명
#define FNAME_OPT "ouput_opt.txt"  // Optimal 알고리즘 결과 저장 파일
#define FNAME_FIFO "ouput_fifo.txt"  // FIFO 알고리즘 결과 저장 파일
#define FNAME_LIFO "output_lifo.txt" // LIFO 알고리즘 결과 저장 파일
#define FNAME_LRU "output_lru.txt"   // LRU 알고리즘 결과 저장 파일
#define FNAME_LFU "output_lfu.txt"   // LFU 알고리즘 결과 저장 파일
#define FNAME_SC "output_sc.txt"     // SC 알고리즘 결과 저장 파일

// 구조체
typedef struct Page
{
	int num;	//page 번호
	int ref;	//참조 횟수
	struct Page * next;
}Page;
typedef struct Deque
{
	Page * page;
	int rear, front;
	int cnt;
}Deque;

// 함수원형
int split_command(char *command, char *seperator, char *argv[]);
void create_random_pstream();
void create_fopen_pstream();
void print_pstream();

void init_deque(Deque *q);					
bool is_empty(Deque *q);					
bool is_full(Deque *q);					
bool deque_search(Deque *q, int find);					
bool add_rear(Deque *q, Page page);		
Page delete_front(Deque *q);				
bool add_front(Deque *q, Page page);		
Page delete_rear(Deque *q);		
void deque_print(Deque *q);	

Page* addToEmpty(Page* last, int num, int ref);	//circularQueue 비어있는 링크드리스트에 Page 추가
Page* addFront(Page* last, int num,int ref);	//circularQueue 맨 앞에 Page 추가
Page* addEnd(Page* last, int num, int ref);		//circularQueue 맨 뒤에 Page 추가
Page* addAfter(Page* last, int num, int item);	//circularQueue 특정 item 뒤에 Page 추가
void deletePage(Page** last, int key);	//circularQueue key값을 가지는 Page 삭제
Page* searchPage(Page* last, int key);	//circularQueue key값을 가지는 Page 리턴
void traverse(Page* last);				//circularQueue 순회

int OPT();
void OPT_pfault(Deque *q, int index, FILE* fp);
void OPT_preplace(Deque *q, int index, FILE* fp);

void FIFO();
void FIFO_pfault(Deque *q, int index, FILE* fp);
void FIFO_preplace(Deque* q,int index,FILE* fp);

void LIFO();
void LIFO_pfault(Deque *q, int index, FILE* fp);
void LIFO_preplace(Deque* q,int index,FILE* fp);

void LRU();
Page* LRU_pfault(Page* last, int index, FILE* fp);
Page* LRU_preplace(Page* last, int index, FILE* fp);

void LFU();
Page* LFU_pfault(Page* last, int index, FILE* fp);
Page* LFU_preplace(Page* last, int index, FILE* fp);

void SC();
Page* SC_pfault(Page* last, int index, FILE* fp);
Page* SC_preplace(Page* last, int index, FILE* fp);


// 전역변수
Page pstream[100000];		// 페이지 스트림
int pstream_size = MIN_PSTREAM;		// 페이지 스트림 크기 (min: 500)
int pframe_num;				// 페이지 프레임 개수
int input_method;			// 데이터 입력 방식
bool isOptimal, isFIFO, isLIFO, isLRU, isLFU, isSC, isESC, isALL;	// 사용자가 선택할 알고리즘
int opt_pfault;				// Optimal 알고리즘 Page Fault 횟수 (비교용)
int cl_size;				// circularQueue 크기


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
	else
		pframe_num++;
	

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
	opt_pfault = OPT();

	if(isALL)
	{
		FIFO();
		LIFO();
		LRU();
		LFU();
		SC();
		//ESC();
	}
	else
	{
		if(isFIFO)
			FIFO();
		
		if(isLIFO)
			LIFO();
		
		if(isLRU)
			LRU();

		if(isLFU)
			LFU();
		
		if(isSC)
			SC();
		
		// if(isESC)
		// 	//ESC();
		
	}



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
	for(int i=0; i<MIN_PSTREAM; i++)
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

// Deque 초기화
void init_deque(Deque *q) 
{
	q->page = (Page*)malloc(sizeof(Page)*(pframe_num));
	q->front = q->rear = 0;
}

// Deque 비어있는지 검사
bool is_empty(Deque *q) 
{
	if (q->front == q->rear) return true;
	else return false;
}

// Deque 가득 찼는지 검사
bool is_full(Deque *q) 
{
	if (((q->rear + 1) % pframe_num) == q->front)
	{
		return true;
	} 
	else return false;
}

// Deque 뒤에 Page 추가
bool add_rear(Deque *q, Page page) 
{
	if (is_full(q)) 
	{
		return false;
	}
	q->rear = (q->rear + 1) % pframe_num;
	q->page[q->rear] = page;
	q->cnt++;
	return true;
}

// Deque 뒤에 요소를 반환 후 삭제
Page delete_rear(Deque *q) 
{
	Page tmp;
	if (!is_empty(q)) 
	{
		tmp = q->page[q->rear];
		q->rear = (q->rear - 1 + pframe_num) % pframe_num;
		q->cnt--;
	}
	return tmp;
}

// Deque 앞에 Page 추가
bool add_front(Deque *q, Page page) 
{
	if (is_full(q)) 
	{
		return false;
	}
	q->page[q->front] = page;
	q->front = (q->front - 1 + pframe_num) % pframe_num;
	q->cnt++;
	return true;
}

// Deque 앞에 Page를 반환
Page get_front(Deque *q) 
{
	Page tmp;
	if (is_empty(q)) 
	{
		return tmp;
	}
	return tmp = q->page[(q->front + 1) % pframe_num];
}

// Deque 앞에 Page를 삭제 후 반환
Page delete_front(Deque *q) 
{
	Page tmp;
	if (!is_empty(q)) 
	{
		tmp = get_front(q);
		q->front = (q->front + 1) % pframe_num;
		q->cnt--;
	}
	return tmp;
}

// Deque 에서 원하는 Page 탐색
bool deque_search(Deque *q, int find) 
{
	// Deque 가 비어있을 경우 -> PageFault
	if (is_empty(q)) 
	{
		return false;
	}

	int i = (q->front + 1) % pframe_num;
	while (i != q->rear) 
	{
		if(q->page[i].num == find)
		{
			return true;
		}
		i = (i + 1) % pframe_num;
	}
	if(q->page[i].num == find)
	{
		return true;
	}

	return false;
}

void deque_print(Deque *q) {
	if(!is_empty(q))
	{
		int i = q->front;
		do
		{
			i = (i+1) % (pframe_num);
			printf("%d | ",q->page[i].num);
		} while (i != q->rear);
		
	}
	printf("\n");
}

Page* addToEmpty(Page* last, int num, int ref) {
  if (last != NULL) return last;

  // allocate memory to the new node
  Page* newNode = (Page*)malloc(sizeof(Page));

  // assign data to the new node
  newNode->num = num;
  newNode->ref = ref;

  // assign last to newNode
  last = newNode;

  // create link to iteself
  last->next = last;

  return last;
}

// add node to the front
Page* addFront(Page* last, int num,int ref) {
  // check if the list is empty
  if (last == NULL) return addToEmpty(last, num,ref);

  // allocate memory to the new node
  Page* newNode = (Page*)malloc(sizeof(Page));

  // add data to the node
  newNode->num = num;
  newNode->ref = ref;

  // store the address of the current first node in the newNode
  newNode->next = last->next;

  // make newNode as head
  last->next = newNode;

  return last;
}

// add node to the end
Page* addEnd(Page* last, int num, int ref) {
  // check if the node is empty
  if (last == NULL) return addToEmpty(last, num,ref);

  // allocate memory to the new node
  Page* newNode = (Page*)malloc(sizeof(Page));

  // add data to the node
  newNode->num = num;
  newNode->ref = ref;

  // store the address of the head node to next of newNode
  newNode->next = last->next;

  // point the current last node to the newNode
  last->next = newNode;

  // make newNode as the last node
  last = newNode;

  return last;
}

// insert node after a specific node
Page* addAfter(Page* last, int num, int item) {
  // check if the list is empty
  if (last == NULL) return NULL;

  Page *newNode, *p;

  p = last->next;
  do {
  // if the item is found, place newNode after it
  if (p->num == item) {
    // allocate memory to the new node
    newNode = (Page*)malloc(sizeof(Page));

    // add data to the node
    newNode->num = num;

    // make the next of the current node as the next of newNode
    newNode->next = p->next;

    // put newNode to the next of p
    p->next = newNode;

    // if p is the last node, make newNode as the last node
    if (p == last) last = newNode;
    return last;
  }

  p = p->next;
  } while (p != last->next);

  printf("\nThe given node is not present in the list");
  return last;
}



// delete a node
void deletePage(Page** last, int key) {
  // if linked list is empty
  if (*last == NULL) return;

  // if the list contains only a single node
  if ((*last)->num == key && (*last)->next == *last) {
  free(*last);
  *last = NULL;
  return;
  }

  Page *temp = *last, *d;

  // if last is to be deleted
  if ((*last)->num == key) {
  // find the node before the last node
  while (temp->next != *last) temp = temp->next;

  // point temp node to the next of last i.e. first node
  temp->next = (*last)->next;
  free(*last);
  *last = temp->next;
  }

  // travel to the node to be deleted
  while (temp->next != *last && temp->next->num != key) {
  temp = temp->next;
  }

  // if node to be deleted was found
  if (temp->next->num == key) {
  d = temp->next;
  temp->next = d->next;
  free(d);
  }
}

Page* searchPage(Page* last, int key)
{
    Page* p = NULL;

    if (last == NULL) 
    {
        return p;
    }

    p = last->next;
    
    do 
    {
        if(p->num == key)
        {
            return p;
        }
        p = p->next;
    } while (p != last->next);

    return p = NULL;
}


void traverse(Page* last) {
  Page* p;

  if (last == NULL) {
  printf("The list is empty");
  return;
  }

  p = last->next;

  do {
  printf("%d(%d) ", p->num,p->ref);
  p = p->next;

  } while (p != last->next);
  printf("\n");
}

int OPT()
{
	// 페이지 프레임 개수 만큼 할당, Deque 초기화
	Deque q;
    init_deque(&q);
	int pfault = 0;


	// 결과를 저장할 파일 open
	FILE *fp;
	if((fp = fopen(FNAME_OPT,"w+")) == NULL)
	{
		fprintf(stderr, "fopen error for %s\n",FNAME_OPT);
		exit(1);
	}

	printf(">> Optimal <<\n");
	fprintf(fp,">> Optimal <<\n");


	// 페이지 스트림 읽기
	for(int i=0; i<pstream_size; i++)
	{	
		// 페이지 프레임 중 원하는 Page 탐색
		if(!deque_search(&q, pstream[i].num))
		{
			//존재하지 않을 경우 -> Page Fault
			pfault++;
			OPT_pfault(&q,i,fp);
		}
		//deque_print(&q);

	}

	//Page Fault 총 횟수 출력
	printf("Total Page Fault: %d\n\n",pfault);
	fprintf(fp,"Total Page Fault: %d\n\n",pfault);
	
	//메모리 해제
	free(q.page);
	fclose(fp);

	return pfault;
}

// Optimal 알고리즘 Page Fault 처리 함수
void OPT_pfault(Deque *q, int index,FILE* fp)
{
	Page new;
	new.num = pstream[index].num;

	//가득 찼을 경우
	if(is_full(q))
	{
		//page replacement
		OPT_preplace(q,index,fp);

	}
	//가득 차지 않았을 경우 
	else
	{
		//deque 에 page 추가
		add_rear(q,new);
		printf("Page Fault: Add Page %d\n",pstream[index].num);
		fprintf(fp,"Page Fault: Add Page %d\n",pstream[index].num);
	}
}

// Optimal Page 교체 함수
void OPT_preplace(Deque *q, int index,FILE* fp)
{
	int longest = -1;
	int longest_index;

	if(!is_empty(q))
	{
		int i = q->front;
		do
		{
			i = (i+1) % (pframe_num);
			int j=index;
			while(j<pstream_size)
			{
				if (q->page[i].num == pstream[j].num)
				{
					break;
				}
				j++;
			}

			if(j>longest)
			{
				longest = j;
				longest_index = i;
			}

		} while (i != q->rear);
		
	}
	
	printf("Page Fault: Replace Page %d -> Page %d\n",q->page[longest_index].num,pstream[index].num);
	fprintf(fp,"Page Fault: Replace Page %d -> Page %d\n",q->page[longest_index].num,pstream[index].num);
	q->page[longest_index] = pstream[index];

}

// FIFO 알고리즘
void FIFO()
{
	// 페이지 프레임 개수 만큼 할당, Deque 초기화
	Deque q;
    init_deque(&q);
	int pfault = 0;


	// 결과를 저장할 파일 open
	FILE *fp;
	if((fp = fopen(FNAME_FIFO,"w+")) == NULL)
	{
		fprintf(stderr, "fopen error for %s\n",FNAME_FIFO);
		exit(1);
	}

	printf(">> FIFO <<\n");
	fprintf(fp,">> FIFO <<\n");


	// 페이지 스트림 읽기
	for(int i=0; i<pstream_size; i++)
	{	
		// 페이지 프레임 중 원하는 Page 탐색
		if(!deque_search(&q, pstream[i].num))
		{
			//존재하지 않을 경우 -> Page Fault
			pfault++;
			FIFO_pfault(&q,i,fp);
		}
		//deque_print(&q);

	}

	//Page Fault 총 횟수 출력
	printf("Total Page Fault: %d\n",pfault);
	fprintf(fp,"Total Page Fault: %d\n",pfault);

	//Optimal 과 비교
	if(pfault > opt_pfault)
	{
		printf("Difference Of Page Fault: FIFO %d > Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: FIFO %d > Optimal %d\n\n",pfault,opt_pfault);
	}
	else if(pfault == opt_pfault)
	{
		printf("Difference Of Page Fault: FIFO %d == Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: FIFO %d == Optimal %d\n\n",pfault,opt_pfault);
	}
	else
	{
		printf("Difference Of Page Fault: FIFO %d < Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: FIFO %d < Optimal %d\n\n",pfault,opt_pfault);
	}
	
	
	//메모리 해제
	free(q.page);
	fclose(fp);

}

// FIFO 알고리즘 Page Fault 처리 함수
void FIFO_pfault(Deque *q, int index,FILE* fp)
{
	Page new;
	new.num = pstream[index].num;

	//가득 찼을 경우
	if(is_full(q))
	{
		//page replacement
		FIFO_preplace(q,index,fp);

	}
	//가득 차지 않았을 경우 
	else
	{
		//deque 에 page 추가
		add_rear(q,new);
		printf("Page Fault: Add Page %d\n",pstream[index].num);
		fprintf(fp,"Page Fault: Add Page %d\n",pstream[index].num);
	}
}

// FIFO Page 교체 함수
void FIFO_preplace(Deque* q,int index,FILE* fp)
{	
	// 처음 들어간 것을 가장 먼저 삭제
	Page del;
	del = delete_front(q);

	// 새로운 Page 추가
	add_rear(q,pstream[index]);

	printf("Page Fault: Replace Page %d -> Page %d\n",del.num,pstream[index].num);
	fprintf(fp,"Page Fault: Replace Page %d -> Page %d\n",del.num,pstream[index].num);
}


// LIFO 알고리즘
void LIFO()
{
	// 페이지 프레임 개수 만큼 할당, Deque 초기화
	Deque q;
    init_deque(&q);
	int pfault = 0;


	// 결과를 저장할 파일 open
	FILE *fp;
	if((fp = fopen(FNAME_LIFO,"w+")) == NULL)
	{
		fprintf(stderr, "fopen error for %s\n",FNAME_LIFO);
		exit(1);
	}

	printf(">> LIFO <<\n");
	fprintf(fp,">> LIFO <<\n");


	// 페이지 스트림 읽기
	for(int i=0; i<pstream_size; i++)
	{	
		// 페이지 프레임 중 원하는 Page 탐색
		if(!deque_search(&q, pstream[i].num))
		{
			//존재하지 않을 경우 -> Page Fault
			pfault++;
			LIFO_pfault(&q,i,fp);
		}
		//deque_print(&q);

	}

	//Page Fault 총 횟수 출력
	printf("Total Page Fault: %d\n",pfault);
	fprintf(fp,"Total Page Fault: %d\n",pfault);

	//Optimal 과 비교
	if(pfault > opt_pfault)
	{
		printf("Difference Of Page Fault: LIFO %d > Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LIFO %d > Optimal %d\n\n",pfault,opt_pfault);
	}
	else if(pfault == opt_pfault)
	{
		printf("Difference Of Page Fault: LIFO %d == Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LIFO %d == Optimal %d\n\n",pfault,opt_pfault);
	}
	else
	{
		printf("Difference Of Page Fault: LIFO %d < Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LIFO %d < Optimal %d\n\n",pfault,opt_pfault);
	}
	
	
	//메모리 해제
	free(q.page);
	fclose(fp);

}

// LIFO 알고리즘 Page Fault 처리 함수
void LIFO_pfault(Deque *q, int index,FILE* fp)
{
	Page new;
	new.num = pstream[index].num;

	//가득 찼을 경우
	if(is_full(q))
	{
		//page replacement
		LIFO_preplace(q,index,fp);

	}
	//가득 차지 않았을 경우 
	else
	{
		//deque 에 page 추가
		add_rear(q,new);
		printf("Page Fault: Add Page %d\n",pstream[index].num);
		fprintf(fp,"Page Fault: Add Page %d\n",pstream[index].num);
	}
}

// LIFO Page 교체 함수
void LIFO_preplace(Deque* q,int index,FILE* fp)
{	
	// 마지막에 들어간 것을 가장 먼저 삭제
	Page del;
	del = delete_rear(q);

	// 새로운 Page 추가
	add_rear(q,pstream[index]);

	printf("Page Fault: Replace Page %d -> Page %d\n",del.num,pstream[index].num);
	fprintf(fp,"Page Fault: Replace Page %d -> Page %d\n",del.num,pstream[index].num);
}

void LRU()
{
	//초기값 설정
	Page* last = NULL;
	int pfault = 0;
	cl_size = 0;



	// 결과를 저장할 파일 open
	FILE *fp;
	if((fp = fopen(FNAME_LRU,"w+")) == NULL)
	{
		fprintf(stderr, "fopen error for %s\n",FNAME_LRU);
		exit(1);
	}

	printf(">> LRU <<\n");
	fprintf(fp,">> LRU <<\n");

	// 페이지 스트림 읽기
	for(int i=0; i<pstream_size; i++)
	{	
		Page * p;
		// 페이지 프레임 중 원하는 Page 탐색
		if((p = searchPage(last, pstream[i].num)) == NULL)
		{
			//존재하지 않을 경우 -> Page Fault
			pfault++;
			last = LRU_pfault(last,i,fp);
		}
		else
		{
			//존재할 경우 -> Hit
			//Page 삭제 후 해당 Page를 상단으로 옮김
			int num = p->num;
			deletePage(&last,num);
			last = addEnd(last,num,0);
		}
		//traverse(last);

	}

	//Page Fault 총 횟수 출력
	printf("Total Page Fault: %d\n",pfault);
	fprintf(fp,"Total Page Fault: %d\n",pfault);

	//Optimal 과 비교
	if(pfault > opt_pfault)
	{
		printf("Difference Of Page Fault: LRU %d > Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LRU %d > Optimal %d\n\n",pfault,opt_pfault);
	}
	else if(pfault == opt_pfault)
	{
		printf("Difference Of Page Fault: LRU %d == Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LRU %d == Optimal %d\n\n",pfault,opt_pfault);
	}
	else
	{
		printf("Difference Of Page Fault: LRU %d < Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LRU %d < Optimal %d\n\n",pfault,opt_pfault);
	}
	
	
	//메모리 해제
	free(last);
	fclose(fp);

}

Page* LRU_pfault(Page* last, int index, FILE* fp)
{

	//가득 찼을 경우
	if(cl_size == pframe_num-1)
	{
		//page replacement
		last = LRU_preplace(last,index,fp);

	}
	//가득 차지 않았을 경우 
	else
	{
		//circularQueue 에 page 추가
		if(last == NULL)
		{	
			// Empty일때
			last = addToEmpty(last,pstream[index].num,0);
			cl_size++;
		}
		else
		{	
			// 값이 존재할때
			last = addEnd(last,pstream[index].num,0);
			cl_size++;
		}
		printf("Page Fault: Add Page %d\n",pstream[index].num);
		fprintf(fp,"Page Fault: Add Page %d\n",pstream[index].num);
	}
	
	return last;
	
}

Page* LRU_preplace(Page* last, int index, FILE* fp)
{
	// 가장 오랫동안 참조되지 않은 페이지 -> last가 가리키는 Page
	Page* del;
	del = last->next;


	printf("Page Fault: Replace Page %d -> Page %d\n",del->num,pstream[index].num);
	fprintf(fp,"Page Fault: Replace Page %d -> Page %d\n",del->num,pstream[index].num);

	// 가장 오랫동안 참조되지 않은 페이지 삭제
	deletePage(&last,del->num);
	

	// 새로운 Page 추가
	last = addEnd(last,pstream[index].num,0);
	
	return last;
}


//LFU 알고리즘
void LFU()
{
	//초기값 설정
	Page* last = NULL;
	int pfault = 0;
	cl_size = 0;



	// 결과를 저장할 파일 open
	FILE *fp;
	if((fp = fopen(FNAME_LFU,"w+")) == NULL)
	{
		fprintf(stderr, "fopen error for %s\n",FNAME_LFU);
		exit(1);
	}

	printf(">> LFU <<\n");
	fprintf(fp,">> LFU <<\n");

	// 페이지 스트림 읽기
	for(int i=0; i<pstream_size; i++)
	{	
		Page * p;
		// 페이지 프레임 중 원하는 Page 탐색
		if((p = searchPage(last, pstream[i].num)) == NULL)
		{
			//존재하지 않을 경우 -> Page Fault
			pfault++;
			last = LFU_pfault(last,i,fp);
		}
		else
		{
			//존재할 경우 -> Hit
			//Page 삭제 후 해당 Page를 상단으로 옮김
			//참조횟수 +1
			int num = p->num;
			int ref = p->ref+1;
			deletePage(&last,num);
			last = addEnd(last,num,ref);
		}
		//traverse(last);

	}

	//Page Fault 총 횟수 출력
	printf("Total Page Fault: %d\n",pfault);
	fprintf(fp,"Total Page Fault: %d\n",pfault);

	//Optimal 과 비교
	if(pfault > opt_pfault)
	{
		printf("Difference Of Page Fault: LFU %d > Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LFU %d > Optimal %d\n\n",pfault,opt_pfault);
	}
	else if(pfault == opt_pfault)
	{
		printf("Difference Of Page Fault: LFU %d == Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LFU %d == Optimal %d\n\n",pfault,opt_pfault);
	}
	else
	{
		printf("Difference Of Page Fault: LFU %d < Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: LFU %d < Optimal %d\n\n",pfault,opt_pfault);
	}
	
	
	//메모리 해제
	free(last);
	fclose(fp);

}

Page* LFU_pfault(Page* last, int index, FILE* fp)
{
	//가득 찼을 경우
	if(cl_size == pframe_num-1)
	{
		//page replacement
		last = LFU_preplace(last,index,fp);

	}
	//가득 차지 않았을 경우 
	else
	{
		//circularQueue 에 page 추가
		if(last == NULL)
		{	
			// Empty일때
			last = addToEmpty(last,pstream[index].num,0);
			cl_size++;
		}
		else
		{	
			// 값이 존재할때
			last = addEnd(last,pstream[index].num,0);
			cl_size++;
		}
		printf("Page Fault: Add Page %d\n",pstream[index].num);
		fprintf(fp,"Page Fault: Add Page %d\n",pstream[index].num);
	}
	
	return last;
}

Page* LFU_preplace(Page* last, int index, FILE* fp)
{
	// 기본값 : 가장 오랫동안 참조되지 않은 페이지 -> last가 가리키는 Page
	Page* del;
	del = last->next;

	
	//	가장 적게 참조되지 않은 페이지 탐색
	Page* p;
	int min_ref = MAX_PSTREAM;
  	p = last->next;
  	do 
	{
		// 참조횟수가 min_ref 보다 작을 경우 -> 삭제 대상
  		if(min_ref > p->ref)
		{
			min_ref = p->ref;
			del = p;
		}
  		p = p->next;

 	} while (p != last->next);
  
	printf("Page Fault: Replace Page %d -> Page %d\n",del->num,pstream[index].num);
	fprintf(fp,"Page Fault: Replace Page %d -> Page %d\n",del->num,pstream[index].num);

	// 가장 오랫동안 참조되지 않은 페이지 삭제
	deletePage(&last,del->num);
	

	// 새로운 Page 추가
	last = addEnd(last,pstream[index].num,0);
	
	return last;
}

void SC()
{
	//초기값 설정
	Page* last = NULL;
	int pfault = 0;
	cl_size = 0;



	// 결과를 저장할 파일 open
	FILE *fp;
	if((fp = fopen(FNAME_SC,"w+")) == NULL)
	{
		fprintf(stderr, "fopen error for %s\n",FNAME_SC);
		exit(1);
	}

	printf(">> SC <<\n");
	fprintf(fp,">> SC <<\n");

	// 페이지 스트림 읽기
	for(int i=0; i<pstream_size; i++)
	{	
		Page * p;
		// 페이지 프레임 중 원하는 Page 탐색
		if((p = searchPage(last, pstream[i].num)) == NULL)
		{
			//존재하지 않을 경우 -> Page Fault
			pfault++;
			last = SC_pfault(last,i,fp);
		}
		else
		{
			//존재할 경우 -> Hit
			//참조비트를 1로 수정
			
			p->ref = 1;
			
		}
		traverse(last);

	}

	//Page Fault 총 횟수 출력
	printf("Total Page Fault: %d\n",pfault);
	fprintf(fp,"Total Page Fault: %d\n",pfault);

	//Optimal 과 비교
	if(pfault > opt_pfault)
	{
		printf("Difference Of Page Fault: SC %d > Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: SC %d > Optimal %d\n\n",pfault,opt_pfault);
	}
	else if(pfault == opt_pfault)
	{
		printf("Difference Of Page Fault: SC %d == Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: SC %d == Optimal %d\n\n",pfault,opt_pfault);
	}
	else
	{
		printf("Difference Of Page Fault: SC %d < Optimal %d\n\n",pfault,opt_pfault);
		fprintf(fp,"Difference Of Page Fault: SC %d < Optimal %d\n\n",pfault,opt_pfault);
	}
	
	
	//메모리 해제
	free(last);
	fclose(fp);

}

Page* SC_pfault(Page* last, int index, FILE* fp)
{
	//가득 찼을 경우
	if(cl_size == pframe_num-1)
	{
		//page replacement
		last = SC_preplace(last,index,fp);

	}
	//가득 차지 않았을 경우 
	else
	{
		//circularQueue 에 page 추가
		if(last == NULL)
		{	
			// Empty일때
			last = addToEmpty(last,pstream[index].num,1);
			cl_size++;
		}
		else
		{	
			// 값이 존재할때
			last = addEnd(last,pstream[index].num,1);
			cl_size++;
		}
		printf("Page Fault: Add Page %d\n",pstream[index].num);
		fprintf(fp,"Page Fault: Add Page %d\n",pstream[index].num);
	}
	
	return last;
}

Page* SC_preplace(Page* last, int index, FILE* fp)
{
	// 기본값 : FIFO -> 처음 들어간 것
	Page* del;
	del = last->next;

	
	//	참조비트 탐색
	Page* p;
	int min_ref = MAX_PSTREAM;
  	p = last->next;
  	do 
	{
		// 참조비트가 1인 경우 
  		if(p->ref == 1)
		{
			// 참조비트를 0으로 변경하고 넘어감
			p->ref = 0;
			p = p->next;
		}
		// 참조비트가 0인 경우
		else
		{
			// 교체 대상 -> break;
			del = p;
			break;
		}
  		
 	} while (p != last->next);
	

	printf("Page Fault: Replace Page %d -> Page %d\n",del->num,pstream[index].num);
	fprintf(fp,"Page Fault: Replace Page %d -> Page %d\n",del->num,pstream[index].num);

	//페이지 삭제
	deletePage(&last,del->num);
	// 새로운 Page 추가
	last = addEnd(last,pstream[index].num,1);
	
	return last;
}