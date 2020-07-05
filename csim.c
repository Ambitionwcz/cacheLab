/*
*Part A:
*In PartA you will write a cache simulator in csim.c that takes a valgrind memory trace as input,
*simulates the hit/miss behavior of a cache memory on this trace,
*and outputs the total number of hits, misses, and evictions.

*StudentNumber:517030910227 
*StudentName:��u��
*/


#include "cachelab.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#define address_length  64//�����ڴ��ַ����Ϊ64

/*cache���*/
typedef struct cache_line //����cacheÿ���һ�У���Чλ����־λ�������滻��use����
{
	char valid;//��Чλ
	unsigned long long int tag;//��־λ
	unsigned long long int use;//����LRU�滻����
} cache_e;

typedef cache_e* cache_s;//ÿ���кܶ���
typedef cache_s* cache_all;//ÿ��cache�кܶ���

cache_all cache;

/*�����и�������*/
int verbose = 0;//�Ƿ�Ҫ��������Ϣ
int s = 0;//cache����
int E = 0;//ÿ������
int b = 0;//2^bΪÿ�п�Ĵ�С���ֽڣ�
char *input_file = NULL;//�����ļ�

/*cache��Ϣ*/
int S;//2^s
int B;//2^b

/*��Ҫ��¼����Ϣ*/
int miss_number = 0;
int hit_number = 0;
int eviction_number = 0;
int num = 0;//Ϊ�����M��������õ�

/*������ȡ��־λ��������*/
unsigned long long int set_mask = 0;
unsigned long long int tag_mask = 0;


/*��ʼ��cache����ҪΪcache����������ڴ�ռ�*/
void init_cache()
{
	cache = (cache_s*)malloc(sizeof(cache_s) * S);
	for (int i = 0; i < S; i++)
	{
		cache[i] = (cache_e*)malloc(sizeof(cache_e) * E);
		for (int j = 0; j < E; j++)
		{
			cache[i][j].valid = 0;
			cache[i][j].tag = 0;
			cache[i][j].use = 0;
		}
	}

	set_mask = (unsigned long long int)(pow(2, s));
	set_mask--;
	int t = address_length - s - b;
	tag_mask = (unsigned long long int)(pow(2, t));
	tag_mask--;
}

void free_cache()
{
	for (int i = 0; i < S; i++)
		free(cache[i]);
	free(cache);
}

void cache_work(unsigned long long int addr)
{
	//��ȡ��־λ��������
	unsigned long long int set = (addr >> b) & set_mask;
	unsigned long long int tag = (addr >> (b + s)) & tag_mask;

	

	int hit = 0;//����Ƿ�����

	for (int i = 0; i < E; i++)
	{
		if (cache[set][i].valid == 1 && cache[set][i].tag == tag)
		{
			hit = 1;
			cache[set][i].use = 0;
		}
		else
			cache[set][i].use++;
	}

	if (hit)
	{
		if (verbose)
			printf("hit");
		hit_number++;
		return;
	}

	if (verbose)
		printf("miss ");
	miss_number++;

	//�滻
	//�ҵ�LRU
	int max = 0;
	int lru_index = 0;

	for (int i = 0; i < E; i++)
	{
		if (cache[set][i].use > max)
		{
			max = cache[set][i].use;
			lru_index = i;
		}
	}

	if (cache[set][lru_index].valid == 1)//����ÿ鲻�ǿյ�
	{
		if (verbose)
			printf("eviction ");
		eviction_number++;
	}
		cache[set][lru_index].valid = 1;
		cache[set][lru_index].tag = tag;
		cache[set][lru_index].use = 0;
	
}

/*��trace�ļ�����*/
void input_trace(char *file)
{
	char tmp[500];
	unsigned long long int addr;
	int len;

	FILE* trace = fopen(file, "r");

	while (fgets(tmp, 500, trace) != NULL)
	{
		if (tmp[1] == 'S' || tmp[1] == 'L' || tmp[1] == 'M')
		{
			sscanf(tmp+3, "%llx,%u", &addr,&len);
			if (verbose)
			{
				printf("%c ", tmp[1]);
				printf("%llx,%u ", addr, len);
			}
			cache_work(addr);

			//MҪ��������
			if (tmp[1] == 'M')
			{
				cache_work(addr);
			}
			if (verbose)
				printf("\n");
		}

	}

	fclose(trace);
}

void printUsage(char* argv[])
{
	printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
	printf("  -h  Optional help flag that prints usage info\n");
	printf("  -v  Optional verbose flag that displays trace info\n");
	printf("  -s <s>: Number of set index bits(S=2^s is the number of sets\n");
	printf("  -E <E>: Associativity(number of lines per set)\n");
	printf("  -b <b>: Number of block bits(B=2^b is the block size)\n");
	printf("  -t <tracefile>: Name of the valgrind trace to replay\n");
	printf("\nFor examples:\n");
	printf("  linux> ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("  linux> ./csim-ref -v -s 4 -E 4 -b 4 -t traces/yi.trace\n");
}

int main(int argc,char* argv[])
{
	char tmp;

	while ((tmp = getopt(argc, argv, "s:E:b:t:vh")) != -1)
	{
		switch (tmp)
		{
		case 'h':
			printUsage(argv);
			break;
		case 'v':
			verbose = 1;
			break;
		case 's':
			s = atoi(optarg);
			break;
		case 'E':
			E = atoi(optarg);
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 't':
			input_file = optarg;
			break;
		}
	}

	S = (unsigned int)(pow(2, s));
	B = (unsigned int)(pow(2, b));

	init_cache();

	input_trace(input_file);

	free_cache();

	printSummary(hit_number, miss_number, eviction_number);
	return 0;
	
}
