# CacheLab
- 实现一个cache模拟器，但只需要计算命中数，未命中数，替换次数
- 采用组相联，即每个cache组有多个行，行替换时采用LRU策略
## 参考资料
[Writeup](http://csapp.cs.cmu.edu/3e/cachelab.pdf) 实验要求及评分标准

[书本内容及实验提示](http://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/recitations/rec07.pdf) 官方ppt，包括cache知识点，对实验的提示
## Cache
- Cache是一个数组，有S=2^s个cache组（set）
- 每个组包含E个cache行（line）
- 每个行是一个数据块（block），大小为B=2^k字节
- cache总大小 = S * B * E
## Cache组织
采用组相联，即每个cache组有多行
## Cache结构体及初始化
```c
struct {
    int valid_bits; // 有效位
    int tag; // 标记
    int stamp; // 时间戳
    // 本实验不要考虑块偏移
}cache_line;
```
cache是一个二维数组
- cache_line cache[S][E];
```c
cache = (cache_line**)malloc(sizeof(cache_line*) * S);
for(int i = 0;i < S;i++) {
    cache[i] = (cache_line*)malloc(sizeof(cache_line) * E);
    for(int j = 0; j < E;j++) {
        cache[i][j].valid_bits = 0;
        cache[i][j].tag = -1;
        cache[i][j].stamp = -1;
    }
}
```
## 实现流程
1. 参数处理：获取s、E、b以及trace文件名；
2. 初始化cache：因为cache是一个动态二维数组，所以需要先进行初始化；
3. 逐行处理trace文件
    
    a. I命令跳过，L和R命令需要调用**命中查询函数**一次，M命令需要查询两次；
    b. 执行完命令后，需要对cache行所有时间戳+1；
4. 关闭文件，释放内存；
## 命中查询
1. 地址解析，获取组索引位和标记位
2. 组内遍历，tag相同表示命中
3. 未命中执行**LRU**策略
```c
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
```
## LRU算法
LRU（Least Recently Used）是一种替换策略，如果缓存不命中，cache从内存中取数据块：

    如果cache有空行，将数据块放入空行，否则替换掉最后一次访问时间最久远的一行（时间戳最大）


