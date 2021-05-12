// meta data cache 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "prefetcher_core.h"
//#include "params.h"
//#include "memory_controller.h"

//extern long long int CYCLE_VAL;

#define SETS 1024
#define WAYS 8
#define OFFSETS 64

#define CACHE_VERBOSE 0

using namespace std;

unsigned int log_base2(unsigned int new_value)
{
    int i;
    for (i = 0; i < 32; i++) {
        new_value >>= 1;
        if (new_value == 0)
            break;
        }
    return i;
}

unsigned long long int tag_2_address(cache_t * my_cache, unsigned long long int in_tag, int set)
{
	return (in_tag << (log_base2(my_cache->num_set) + log_base2(my_cache->num_offset))) + (set << log_base2(my_cache->num_offset));
}

unsigned long long int tcp_tag_2_address(unsigned long long int in_tag, int set, int llc_sets)
{
    return (in_tag << (log_base2(llc_sets) + log_base2(OFFSETS))) + (set << log_base2(OFFSETS));
}

// to malloc a new cache and initialize it 
cache_t * init_cache(int set, int way, int offset)
{
	cache_t * my_cache = (cache_t *)malloc(sizeof(cache_t));
	my_cache->miss_num = 0;
	my_cache->hit_num = 0;
	my_cache->num_offset = offset;
	my_cache->num_way = way;
	my_cache->num_set = set;
	tag_t * my_tag = (tag_t *)malloc(sizeof(tag_t)*set*way);
	for (int i=0; i<set*way; i++)
	{
		my_tag[i].address = 0x0;
		my_tag[i].lru = i%way;  		 // 0 to way-1
		my_tag[i].dirty = 0;    		 // not dirty
		my_tag[i].valid = 0;   			 // invalid
		my_tag[i].father_not_update = 0; // update!
		my_tag[i].instruction_id = 0;
		my_tag[i].thread_id = 0;
	}
	my_cache->tag_array = my_tag;
	
	return (my_cache);
}

// to look up inside the cache, if it finds an invalid cache line,
// it returns it back but if it can not find it at all it returns NULL
// do_update is 1 means this access should be counted and if it is 0 
// this access is not counted as an official access
// if access is 1 it is a write request looking for this and if access
// is 0 the request is a read looking for this cache line 
tag_t * look_up (cache_t * my_cache, unsigned long long int addr, int do_update, optype_t access)
{
	//printf ("loooook up \n");
	unsigned long long int this_tag = find_tag(my_cache->num_offset, my_cache->num_set, addr);
	unsigned long long int this_set = find_set(my_cache->num_offset, my_cache->num_set, addr);
	int hit = 0;
	int found; // index of cache line 
	//print_cache(cnt_cache);
	for (int i=(this_set*my_cache->num_way);i<(this_set*my_cache->num_way+my_cache->num_way); i++)
	{
		if ((my_cache->tag_array[i].address == this_tag) && (my_cache->tag_array[i].valid == 1))
		{
			// cache hit
			found = i;
			hit = 1;
			break;
		}
	}
	if (hit == 1)
	{
		if (do_update)
		{
			if (access == WRITE)
				my_cache->tag_array[found].dirty = 1;
			my_cache->hit_num++;
			if (CACHE_VERBOSE)
				printf ("hit for %llx tag = %llx \n", addr, find_tag(my_cache->num_offset, my_cache->num_set, addr));
			for (int i=(this_set*my_cache->num_way);i<(this_set*my_cache->num_way+my_cache->num_way); i++)
			{
				if ((my_cache->tag_array[i].lru <= my_cache->tag_array[found].lru) && (i != found))
					my_cache->tag_array[i].lru++;
			}
			my_cache->tag_array[found].lru = 0;
		}
		return(&my_cache->tag_array[found]);
	}
	else
	{
		if (do_update)
		{
			my_cache->miss_num++;
			if (CACHE_VERBOSE)
				printf ("miss for %llx tag = %llx\n", addr, find_tag(my_cache->num_offset, my_cache->num_set, addr));
		}
		return (NULL);
	}
}

//this function return the tag which is the candidate to be replaced
// based on LRU policy 
// father_not_update shows if its father has updated or not 
int  replacement_cache (cache_t * my_cache, unsigned long long int addr)
{
	int this_set = find_set(my_cache->num_offset, my_cache->num_set, addr);
	int index = 0;
	for (int i = this_set*my_cache->num_way; i<(this_set*my_cache->num_way+my_cache->num_way); i++)
	{
		if (my_cache->tag_array[index].lru <= my_cache->tag_array[i].lru)
		{
			index  = i;
		}
	}
	
	return(index);
}

// to insert a cache line into a cache, access shows it is a write request
// or a read one. it returns the cache line which is replaced 
// father_not_update shows if its father has updated or not 
tag_t * insert_cache (cache_t * my_cache, unsigned long long int addr, int father_not_update,
													int thread_id, int instruction_id, optype_t access)
{
	tag_t * ltag = look_up (my_cache, addr, 0, access);
	//if (ltag != NULL)
	
	/*if (find_tag(my_cache->num_offset, my_cache->num_set,addr) == 0x5100009)
	{
		printf ("%lld | addr = %llx \n", CYCLE_VAL,addr);	
	}*/

	
	assert (ltag == NULL);
	int this_set = find_set(my_cache->num_offset, my_cache->num_set, addr);
	/*printf ("inserting %llx in set = %d \n", addr, this_set);
	for (int i=0; i<my_cache->num_way; i++)
	{
		printf ("%llx ", my_cache->tag_array[this_set*my_cache->num_way+i].address);
	}
	printf ("\n");
	*/
	int index = replacement_cache (my_cache, addr);
	//printf ("address = %llx this_set = %d index = %d \n", addr, this_set, index);
	tag_t * replaced = (tag_t *)malloc(sizeof(tag_t));
	replaced->address = my_cache->tag_array[index].address;
	replaced->lru = my_cache->tag_array[index].lru;
	replaced->valid = my_cache->tag_array[index].valid;
	replaced->dirty = my_cache->tag_array[index].dirty;
	replaced->father_not_update = my_cache->tag_array[index].father_not_update;
	replaced->thread_id = my_cache->tag_array[index].thread_id;
	replaced->instruction_id = my_cache->tag_array[index].instruction_id;
	my_cache->tag_array[index].address = find_tag(my_cache->num_offset,my_cache->num_set, addr);
	for (int i=(this_set*my_cache->num_way);i<(this_set*my_cache->num_way+my_cache->num_way); i++)
	{
		if ((my_cache->tag_array[i].lru <= my_cache->tag_array[index].lru) && (i != index))
			my_cache->tag_array[i].lru++;
	}
	my_cache->tag_array[index].lru = 0;
	my_cache->tag_array[index].valid = 1;
	my_cache->tag_array[index].father_not_update = father_not_update;
	if (access == WRITE)
		my_cache->tag_array[index].dirty = 1;
	else
		my_cache->tag_array[index].dirty = 0;
	/*	
	printf ("==============================after \n");
	for (int i=0; i<my_cache->num_way; i++)
	{
		printf ("%llx ", my_cache->tag_array[this_set*my_cache->num_way+i].address);
	}
	printf ("\n");*/
	
	/*if (find_tag(my_cache->num_offset, my_cache->num_set,addr) == 0x5100009)
	{
		//printf ("%lld | addr = %llx \n", CYCLE_VAL,addr);
		print_cache(hash_cache);
	}*/
		
   return (replaced);
}

void print_cache (cache_t * my_cache)
{
	printf ("================================= cache content ===============\n");
	for (int i=0; i<(my_cache->num_way*my_cache->num_set); i++)
	{
		int prnt;
		if (i%my_cache->num_way==0)
		{
			prnt = 0;
			for (int t=i; t<i+my_cache->num_way; t++)
			{
				if (my_cache->tag_array[t].valid)
				{
					prnt = 1;
					break;
				}
			}
			if (prnt == 1)
				printf("\nset %d: ", i/my_cache->num_way);
		}
		if (my_cache->tag_array[i].valid)
		{
			printf ("[%d %llx]", i%my_cache->num_way, my_cache->tag_array[i].address);
		}
		
	}
	printf ("\n===============================================================\n");
	
}

int find_pht_index(list<unsigned long long int> &tht_set, int index, int m, int n, int scheme) {

    int pht_index;
    size_t hash_output;
    stringstream s;
    hash<string> hash_fn;
    long long int tag_sum = 0;
    
    for(list<unsigned long long int>::iterator it=tht_set.begin(); it!=tht_set.end(); it++) {
        if(scheme == 0 || scheme == 1) {
            tag_sum += *it;
        }
        else if(scheme == 2 || scheme == 3) {
            s << hex << *it;
        }
    }
    
    if(scheme == 0) {
        pht_index = (int) (((tag_sum & (((long long int)0x1 << m) -1)) << n) | (index & (((long long int)0x1 << n) -1)));
    }
    else if(scheme == 1) {
        pht_index = (int) (tag_sum & (((long long int)0x1 << (m+n)) -1));
    }
    else if(scheme == 2) {
        hash_output = hash_fn(s.str());
        pht_index = (int) ((((long long int)hash_output & (((long long int)0x1 << m) -1)) << n) | (index & (((long long int)0x1 << n) -1)));   
    }
    else if(scheme == 3) {
        hash_output = hash_fn(s.str());
        pht_index = (int) ((long long int)hash_output & (((long long int)0x1 << (m+n)) -1));
    }
    
    return pht_index;
}

unsigned long long int find_pht_tag(list<unsigned long long int> &tht_set, int scheme) {
    
    unsigned long long int pht_tag;

    if(scheme == 0) {
        pht_tag = tht_set.front();
    }

    return pht_tag;
}

void update_tables(vector<list<unsigned long long int> > &tht_table, vector<list<pht_entry> > &pht_table, 
                                         int pht_ways,
                                         int ld_miss_index, unsigned long long int ld_miss_tag,
                                         int m, int n, 
                                         int tht_ordering_scheme, int pht_index_scheme, int pht_tag_scheme) {

    int pht_index = find_pht_index(tht_table[ld_miss_index], ld_miss_index, m, n, pht_index_scheme);
    unsigned long long int pht_tag = find_pht_tag(tht_table[ld_miss_index], pht_tag_scheme);
    bool pht_entry_found = false;

    for (list<pht_entry>::iterator it=pht_table[pht_index].begin(); it!=pht_table[pht_index].end(); it++) {

        if(it->tag == pht_tag) {
            pht_entry_found = true;
            pht_table[pht_index].erase(it);
            break;
        }    
    }

    if(!pht_entry_found && pht_table.size() == pht_ways) {
        pht_table[pht_index].pop_back();
    }
    
    pht_entry new_entry = {pht_tag, ld_miss_tag};
    pht_table[pht_index].push_front(new_entry);

    if(tht_ordering_scheme == 0) {
        tht_table[ld_miss_index].pop_back();
        tht_table[ld_miss_index].push_front(ld_miss_tag);
    }
    else if(tht_ordering_scheme == 1) {
        if(tht_table[ld_miss_index].front() != ld_miss_tag) {
            tht_table[ld_miss_index].pop_back();
            tht_table[ld_miss_index].push_front(ld_miss_tag);
        }
    }
    else if(tht_ordering_scheme == 2) {
        bool tht_entry_found = false;
        for (list<unsigned long long int>::iterator it=tht_table[ld_miss_index].begin(); it!=tht_table[ld_miss_index].end(); it++) {
            if(*it == ld_miss_tag) {
                tht_table[ld_miss_index].erase(it);
                tht_entry_found = true;
                break;
            }
        }
        if(!tht_entry_found) 
            tht_table[ld_miss_index].pop_back();

        tht_table[ld_miss_index].push_front(ld_miss_tag);
    }
}

unsigned long long int lookup_tables(vector<list<unsigned long long int> > &tht_table, vector<list<pht_entry> > &pht_table,
                                     int ld_miss_index, unsigned long long int ld_miss_tag,
                                     int m, int n, 
                                     int pht_index_scheme, int pht_tag_scheme, int replacement_scheme, int llc_sets) {

    int pht_index = find_pht_index(tht_table[ld_miss_index], ld_miss_index, m, n, pht_index_scheme);
    unsigned long long int prefetch_addr = 0;
    bool pht_entry_found = false;


    for (list<pht_entry>::iterator it=pht_table[pht_index].begin(); it!=pht_table[pht_index].end(); it++) {
        if(it->tag == ld_miss_tag) {
            pht_entry_found = true;
            prefetch_addr = it->tagb;
            if(replacement_scheme) {
                pht_table[pht_index].erase(it);
            }
            break;
        }    
    }
    
    if(pht_entry_found) {
        if(replacement_scheme) {
            pht_entry new_entry = {ld_miss_tag, prefetch_addr};
            pht_table[pht_index].push_front(new_entry);
        }
        prefetch_addr = tcp_tag_2_address(prefetch_addr, ld_miss_index, llc_sets);
    }

    return prefetch_addr;
}

int main(int argc, char** argv) {

    fstream trace_file;
    trace_file.open(argv[1], ios::in);
    string trace_line;
    const char* trace_entry;
    unsigned long long int unq_inst_id;
    unsigned long long int cyc_cnt;
    unsigned long long int ld_addr;
    unsigned long long int ld_pc;
    int llc_hit;
    int miss_index;
    unsigned long long int miss_tag;
    int tht_ways = atoi(argv[2]); //tag history length
    int pht_ways = atoi(argv[3]);
    int m = atoi(argv[4]);
    int n = atoi(argv[5]);
    int tht_ordering_scheme = atoi(argv[6]);
    int pht_index_scheme = atoi(argv[7]);
    int pht_tag_scheme = atoi(argv[8]);
    int replacement_scheme = atoi(argv[9]);
    int llc_sets = atoi(argv[10]);
    unsigned long long int prefetch_addr;
    int pht_sets = 1 << (m+n);

    vector<list<unsigned long long int> > tht(llc_sets, list<unsigned long long int>(tht_ways, 0));
    vector<list<pht_entry> > pht(pht_sets, list<pht_entry>()); 

    while(getline(trace_file, trace_line)) {

        trace_entry = trace_line.c_str();

        if (sscanf(trace_entry,"%Lu, %Lu, %Lx, %Lx, %d", &unq_inst_id, &cyc_cnt, &ld_addr, &ld_pc, &llc_hit) < 1) {
            cout << "Panic.  Poor trace format.\n";
            return -1;            
        }

        miss_index = find_set(OFFSETS, llc_sets, ld_addr);
        miss_tag = find_tag(OFFSETS, llc_sets, ld_addr);
        
        update_tables(tht, pht, pht_ways, miss_index, miss_tag, m, n, tht_ordering_scheme, pht_index_scheme, pht_tag_scheme);
        prefetch_addr = lookup_tables(tht, pht, miss_index, miss_tag, m, n, pht_index_scheme, pht_tag_scheme, replacement_scheme, llc_sets);
        
        if(prefetch_addr) {
            cout << dec << unq_inst_id << " " << hex << prefetch_addr << endl;
        }
    }
    trace_file.close(); 
    return 0;
}
