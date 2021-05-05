#ifndef __META_CACHE_H__
#define __META_CACHE_H__

//#include "memory_controller.h"

typedef struct pht 
{
    unsigned long long int tag;
    unsigned long long int tagb;
}pht_entry;

//int find_pht_index(vector<unsigned long long int> &tht_set, int index, int m, int n, int scheme);

//unsigned long long int find_pht_tag(vector<unsigned long long int> &tht_set, int tht_tail, int scheme);

//unsigned long long int update_tables(vector<vector<unsigned long long int> > &tht_table, multimap<int, pht_entry> &pht_table, vector<int> &tht_table_tails,
//                                     int thl, int ld_miss_index, unsigned long long int ld_miss_tag, int m, int n, int pht_index_scheme, int pht_tag_scheme);

//unsigned long long int lookup_tables();


typedef struct tag
{
  unsigned long long int address;
  int lru;
  int dirty;
  int valid;
  int thread_id;
  int instruction_id;
  int father_not_update;
}tag_t;

typedef struct cache
{
	int num_set;
	int num_way;
	int num_offset;
	double hit_num;
	double miss_num;
	tag_t *tag_array;
	
}cache_t;


#define find_tag(numoffset, numset, addr) ((addr) >> (log_base2(numoffset) + log_base2(numset)))
#define find_set(numoffset, numset, addr) (((addr) >> log_base2(numoffset)) & (((long long int)0x1 <<log_base2(numset)) -1))

typedef enum {READ, WRITE} optype_t;

cache_t *cnt_cache;

// convert tag to address 
unsigned long long int tag_2_address(cache_t * my_cache, unsigned long long int in_tag, int set);

// to initialize a cache 
cache_t * init_cache (int set, int way, int offset);

// to look up the data inside the cache 
tag_t* look_up (cache_t * my_cache, unsigned long long int addr, int do_update, optype_t access);

//to replace a cache line and return back the replaced cache line  
int  replacement_cache (cache_t * my_cache, unsigned long long int addr);

//to insert a cache line in cache 
tag_t * insert_cache (cache_t * my_cache, unsigned long long int addr, int father_not_update,
													int thread_id, int instruction_id, optype_t access);
													
													
void print_cache (cache_t * my_cache);


#endif //__META_CACHE_H__
