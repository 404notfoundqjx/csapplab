#include "cachelab.h"
#include <stdlib.h>
#include <unistd.h> 
#include <getopt.h> //use getopt
#include <limits.h>
#include <string.h>

int h, v, s, E, b, S; // 模拟的参数
int hit_count, miss_count, eviction_count; // printSummary函数中的参数
char t[1000]; // trace文件名

typedef struct {
    int valid_bits; // 合法位
    int tag;        // 标记为
    int stamp;      // 时间戳
}cache_line, *cache_asso, **cache;

cache _cache_ = NULL;

// 打印 helper 内容的函数，-h 命令使用，内容可自定义
void printUsage()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
            "Options:\n"
            "  -h         Print this help message.\n"
            "  -v         Optional verbose flag.\n"
            "  -s <num>   Number of set index bits.\n"
            "  -E <num>   Number of lines per set.\n"
            "  -b <num>   Number of block offset bits.\n"
            "  -t <file>  Trace file.\n\n"
            "Examples:\n"
            "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
            "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void init_cache() {
    _cache_ = (cache)malloc(sizeof(cache_asso) * S);
    for(int i = 0;i < S;i++) {
        _cache_[i] = (cache_asso)malloc(sizeof(cache_line) * E);
        for(int j = 0; j < E;j++) {
            _cache_[i][j].valid_bits = 0;
            _cache_[i][j].tag = -1;
            _cache_[i][j].stamp = -1;
        }
    }
}

void update(unsigned int address) {
    // 对地址进行解析
    //获取组索引位和标记位
    int setindex_add = (address >> b) & ((-1U) >> (64-s));
    int tag_add = address >> (b + s);

    int max_stamp = INT_MIN;
    int max_stamp_index = -1;
    
    //组内遍历，tag相同表示命中
    for(int i = 0; i < E;i++) {
        if(_cache_[setindex_add][i].tag == tag_add) {
            _cache_[setindex_add][i].stamp = 0;
            ++hit_count;
            return;
        }
    }

    // 未命中，先找空行
    for(int i = 0;i < E;i++) {
        if(_cache_[setindex_add][i].valid_bits == 0) {
            _cache_[setindex_add][i].valid_bits = 1;
            _cache_[setindex_add][i].tag = tag_add;
            _cache_[setindex_add][i].stamp = 0;
            miss_count++;
            return;
        }
    }
    eviction_count++;
    miss_count++;

    //没有空行，执行替换策略
    //LRU
    for(int i = 0;i < E;i++) {
        if(_cache_[setindex_add][i].stamp > max_stamp) {
            max_stamp = _cache_[setindex_add][i].stamp;
            max_stamp_index = i;
        }
    }
    _cache_[setindex_add][max_stamp_index].tag = tag_add;
    _cache_[setindex_add][max_stamp_index].stamp = 0;
    return;
}

void update_stamp() {
	for(int i = 0; i < S; ++i) 
		for(int j = 0; j < E; ++j)
			if(_cache_[i][j].valid_bits == 1)
				++_cache_[i][j].stamp;
}

void parse_trace() {
    FILE* fp = fopen(t, "r");
    if(fp == NULL) {
        printf("open error");
        exit(-1);
    }

    char operation;
    unsigned int address;
    int size;
    while(fscanf(fp, " %c %xu,%d\n", &operation, &address, &size) > 0) {
        switch(opertion) {
            case "I": continue;
            case "L":
                update(address);
                break;
            // M表示数据修改，每次修改，需要进行一次L和一次R
            case "M":
                update(address);
            case "S":
                update(address);
        }
        //执行完一个指令后，所有时间戳+1
        update_stamp();
    }

    fclose(fp);
    for(int i = 0 ;i < S;i++) {
        free(_cache_[i]);
    }
    free(_cache_);
}

int main(int argc, char* argv[]) {
    h = 0;
    v = 0;
    hit_count = miss_count = eviction_count = 0;
    int opt; // 接收getopt的返回值

    // 不可省略参数后面要加冒号
    while(-1 != (opt = (getopt(argc, argv, "hvs:E:b:t:")))) {
        switch(opt) {
            case 'h':
                h = 1;
                printUsage();
                break;
            case 'v':
                v = 1;
                printUsage();
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
                strcpy(t, optarg);
                break;
            default:
                printUsage();
                break;
        }
    }

    if(s<=0 || E<=0 || b<=0 || t==NULL) return -1;
    // S=2^s
    S = 1 << s;
    FILE* fp = fopen(t, "r");
    if(fp==NULL) {
        printf("open error");
        exit(-1);
    }

    init_cache();
    parse_trace();

    printSummary(0, 0, 0);
    return 0;
}
