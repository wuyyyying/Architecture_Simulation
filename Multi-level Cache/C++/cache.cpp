#include "cache.hpp"

// Use this space for declaring any global variables that you might need

int64_t c, s, C, S, b, v, k;
int64_t L1_ways, L1_sets, L2_ways, L2_sets;
int64_t L1_tag, L1_index, vic_tag, L2_tag, L2_index;

typedef struct L1_set {
		int64_t* counter;
		int64_t* tag;
		int64_t* valid;
		int64_t* dirty;
} L1_set;

typedef struct L1_cache {
		L1_set* sets;
} L1_cache;

L1_cache L1;

typedef struct L2_set {
		int64_t* counter;
		int64_t* tag;
		int64_t* valid;
		int64_t* dirty;
		int64_t* prefetch;
} L2_set;

typedef struct L2_cache {
		L2_set* sets;
} L2_cache;

L2_cache L2;

typedef struct victim {
		int64_t* counter;
		int64_t* tag;
		int64_t* valid;
		int64_t* dirty;
} victim;

victim vic;

void install_to_L1(int64_t isDirty, int64_t tag, int64_t index, struct cache_stats_t *stats);
void evict_to_vic(int64_t isDirty, int64_t tag, struct cache_stats_t *stats);
void install_to_L2(int64_t isDirty, int64_t tag, int64_t index, struct cache_stats_t *stats);
void evict_to_L2(int64_t isDirty, int64_t tag, int64_t index, struct cache_stats_t *stats);
int64_t L1_hit();
int64_t vic_hit();
int64_t L2_hit(struct cache_stats_t *stats);
void prefetch(int64_t tag, int64_t index, struct cache_stats_t *stats);
void install_to_L1_no(int64_t isDirty, int64_t tag, int64_t index, struct cache_stats_t *stats);



/** @brief Function to initialize your cache structures and any globals that you might need
 *
 *  @param conf pointer to the cache configuration structure
 *
 */
void cache_init(struct cache_config_t *conf)
{
  c = int64_t(conf->c);
  s = int64_t(conf->s);
  C = int64_t(conf->C);
  S = int64_t(conf->S);
  b = int64_t(conf->b);
  v = int64_t(conf->v);
  k = int64_t(conf->k);

  L1_ways = 1 << s;
  L1_sets = 1 << (c - s - b);
  L2_ways = 1 << S;
  L2_sets = 1 << (C - S - b);

  L1.sets = new L1_set[L1_sets];

  for (int64_t i = 0; i < L1_sets; i++) {
      (L1.sets)[i].counter = new int64_t[L1_ways];
      (L1.sets)[i].tag = new int64_t[L1_ways];
      (L1.sets)[i].valid = new int64_t[L1_ways];
      (L1.sets)[i].dirty = new int64_t[L1_ways];

      for (int64_t j = 0; j < L1_ways; j++) {
          (L1.sets)[i].counter[j] = 0;
          (L1.sets)[i].tag[j] = 0;
          (L1.sets)[i].valid[j] = 0;
          (L1.sets)[i].dirty[j] = 0;
      }
  }

  L2.sets = new L2_set[L2_sets];

  for (int64_t i = 0; i < L2_sets; i++) {
      (L2.sets)[i].counter = new int64_t[L2_ways];
      (L2.sets)[i].tag = new int64_t[L2_ways];
      (L2.sets)[i].valid = new int64_t[L2_ways];
      (L2.sets)[i].dirty = new int64_t[L2_ways];
      (L2.sets)[i].prefetch = new int64_t[L2_ways];

      for (int64_t j = 0; j < L2_ways; j++) {
          (L2.sets)[i].counter[j] = 0;
          (L2.sets)[i].tag[j] = 0;
          (L2.sets)[i].valid[j] = 0;
          (L2.sets)[i].dirty[j] = 0;
          (L2.sets)[i].prefetch[j] = 0;
      }
  }

  vic.counter = new int64_t[v];
  vic.tag = new int64_t[v];
  vic.valid = new int64_t[v];
  vic.dirty = new int64_t[v];

  for (int64_t i = 0; i < v; i++) {
      vic.tag[i] = 0;
      vic.valid[i] = 0;
      vic.dirty[i] = 0;
      vic.counter[i] = 0;
  }
}

/** @brief Function to initialize your cache structures and any globals that you might need
 *
 *  @param addr The address being accessed
 *  @param rw Tell if the access is a read or a write
 *  @param stats Pointer to the cache statistics structure
 *
 */
void cache_access(uint64_t addr, char rw, struct cache_stats_t *stats)
{
  stats->num_accesses++;
  if (rw == 'R') {
      stats->num_accesses_reads++;
  } else {
      stats->num_accesses_writes++;
  }

  L1_tag = int64_t((addr >> (c - s))) & ((1 << (64 - c + s)) - 1);
  L1_index = int64_t((addr >> b)) & ((1 << (c - b - s)) - 1);

  vic_tag = int64_t((addr >> b)) & ((1 << (64 - b)) - 1);

  L2_tag = int64_t((addr >> (C - S))) & ((1 << (64 - C + S)) - 1);
  L2_index = int64_t((addr >> b)) & ((1 << (C - b - S)) - 1);

  for (int64_t i = 0; i < L1_ways; i++) {
      (L1.sets)[L1_index].counter[i]++;
  }

  for (int64_t i = 0; i < L2_ways; i++) {
      (L2.sets)[L2_index].counter[i]++;
  }

  int64_t flag1 = L1_hit();

  if (flag1 != -1) { // read/write hit in L1
      (L1.sets)[L1_index].tag[flag1] = L1_tag;
      (L1.sets)[L1_index].valid[flag1] = 1;
      int64_t min = 9999999999;
      for (int64_t i = 0; i < L1_ways; i++) {
          if ((L1.sets)[L1_index].counter[i] < min && (L1.sets)[L1_index].valid[i] == 1) {
              min = (L1.sets)[L1_index].counter[i];
          }
      }

      (L1.sets)[L1_index].counter[flag1] = min - 1; // MRU

      if (rw == 'W') {
          (L1.sets)[L1_index].dirty[flag1] = 1;
      }
  } else { // read/write miss in L1
      stats->num_misses_l1++;
      if (rw == 'R') {
          stats->num_misses_reads_l1++;
      } else {
          stats->num_misses_writes_l1++;
      }

      if (v == 0) { // no vic
          stats->num_misses_vc++;
          if (rw == 'R') {
              stats->num_misses_reads_vc++;
          } else {
              stats->num_misses_writes_vc++;
          }

          int64_t Flag = L2_hit(stats);

          if (Flag != -1) { // read/write hit in L2
              int64_t min = 9999999999;
              for (int64_t i = 0; i < L2_ways; i++) {
                  if ((L2.sets)[L2_index].counter[i] < min && (L2.sets)[L2_index].valid[i] == 1) {
                      min = (L2.sets)[L2_index].counter[i];
                  }
              }

              (L2.sets)[L2_index].counter[Flag] = min - 1; // MRU
              //(L2.sets)[L2_index].valid[Flag] = 1;
              //(L2.sets)[L2_index].tag[Flag] = L2_tag;

              if (rw == 'W') {
                  install_to_L1_no(1, L1_tag, L1_index, stats);
              } else {
                  install_to_L1_no((L2.sets)[L2_index].dirty[Flag], L1_tag, L1_index, stats);
              }

          } else { // read/write miss in L2
              stats->num_misses_l2++;
              if (rw == 'R') {
                  stats->num_misses_reads_l2++;
              } else {
                  stats->num_misses_writes_l2++;
              }

              install_to_L2(0, L2_tag, L2_index, stats);

              if (rw == 'W') {
                  install_to_L1_no(1, L1_tag, L1_index, stats);
              } else {
                  install_to_L1_no(0, L1_tag, L1_index, stats);
              }

              // prefetch
              for (int64_t i = 1; i <= k ; i++) {
                uint64_t temp = addr + uint64_t((1 << b) * i);
                int64_t Tag = int64_t((temp >> (C - S))) & ((1 << (64 - C + S)) - 1);
                int64_t Index = int64_t(temp >> b) & ((1 << (C - b - S)) - 1);
                prefetch(Tag, Index, stats);
              }

          }
          return;
      }




      int64_t flag2 = vic_hit();

      if (flag2 != -1) { // read/write hit in vic
          stats->num_hits_vc++;
          // LRU of L1
          int64_t max = -9999999999;
          int64_t temp = -1;
          for (int64_t i = 0; i < L1_ways; i++) {
              if ((L1.sets)[L1_index].counter[i] > max && (L1.sets)[L1_index].valid[i] == 1) {
                  max = (L1.sets)[L1_index].counter[i];
                  temp = i;
              }
          }

          // bookkeeping
          int64_t Tag_L1_to_vic = ((L1.sets)[L1_index].tag[temp] << (c - s - b)) + L1_index;
          int64_t Dirty_L1_to_vic = (L1.sets)[L1_index].dirty[temp];


          int64_t min = 9999999999;
          for (int64_t i = 0; i < L1_ways; i++) {
              if ((L1.sets)[L1_index].counter[i] < min && (L1.sets)[L1_index].valid[i] == 1) {
                  min = (L1.sets)[L1_index].counter[i];
              }
          }

          (L1.sets)[L1_index].tag[temp] = L1_tag;
          (L1.sets)[L1_index].counter[temp] = min - 1;
          (L1.sets)[L1_index].valid[temp] = 1;

          if (rw == 'W') {
              (L1.sets)[L1_index].dirty[temp] = 1;
          } else {
              (L1.sets)[L1_index].dirty[temp] = vic.dirty[flag2];
          }

          int64_t Min = 9999999999;
          for (int64_t i = 0; i < v; i++) {
              if (vic.counter[i] < Min && vic.valid[i] == 1) {
                  Min = vic.counter[i];
              }
          }

          vic.tag[flag2] = Tag_L1_to_vic;
          vic.dirty[flag2] = Dirty_L1_to_vic;
          vic.valid[flag2] = 1;
          vic.counter[flag2] = Min - 1;


      } else { // read/write miss in vic




          stats->num_misses_vc++;
          if (rw == 'R') {
              stats->num_misses_reads_vc++;
          } else {
              stats->num_misses_writes_vc++;
          }
          int64_t flag3 = L2_hit(stats);

          if (flag3 != -1) { // read/write hit in l2
              int64_t min = 9999999999;
              for (int64_t i = 0; i < L2_ways; i++) {
                  if ((L2.sets)[L2_index].counter[i] < min && (L2.sets)[L2_index].valid[i] == 1) {
                      min = (L2.sets)[L2_index].counter[i];
                  }
              }

              (L2.sets)[L2_index].counter[flag3] = min - 1; // MRU

              if (rw == 'W') {
                  install_to_L1(1, L1_tag, L1_index, stats);
              } else {
                  install_to_L1((L2.sets)[L2_index].dirty[flag3], L1_tag, L1_index, stats);
              }
          } else { // read/write miss in l2
              stats->num_misses_l2++;
              if (rw == 'R') {
                  stats->num_misses_reads_l2++;
              } else {
                  stats->num_misses_writes_l2++;
              }
              install_to_L2(0, L2_tag, L2_index, stats);

              if (rw == 'W') {
                  install_to_L1(1, L1_tag, L1_index, stats);
              } else {
                  install_to_L1(0, L1_tag, L1_index, stats);
              }

              // prefetch
              for (int64_t i = 1; i <= k; i++) {
                uint64_t temp = addr + uint64_t((1 << b) * i);
                int64_t Tag = int64_t((temp >> (C - S))) & ((1 << (64 - C + S)) - 1);
                int64_t Index = int64_t(temp >> b) & ((1 << (C - b - S)) - 1);
                prefetch(Tag, Index, stats);
              }
          }
      }
  }
}

void install_to_L1_no(int64_t isDirty, int64_t tag, int64_t index, struct cache_stats_t *stats) { // MRU

	for (int64_t i = 0; i < L1_ways; i++) { // find empty space
		if ((L1.sets)[index].valid[i] == 0) {

			int64_t min = 9999999999;
			for (int64_t j = 0; j < L1_ways; j++) {
				if ((L1.sets)[index].counter[j] < min && (L1.sets)[index].valid[j] == 1) {
					min = (L1.sets)[index].counter[j];
				}
			}
			(L1.sets)[index].valid[i] = 1;
	  		(L1.sets)[index].tag[i] = tag;
			(L1.sets)[index].dirty[i] = isDirty;
			(L1.sets)[index].counter[i] = min - 1; // MRU

			return;
		}
	}

	// full
	int64_t max = -9999999999;
	int64_t temp = -1;
	for (int64_t i = 0; i < L1_ways; i++) {
		if ((L1.sets)[index].counter[i] > max && (L1.sets)[index].valid[i] == 1) {
			max = (L1.sets)[index].counter[i];
			temp = i;
		}
	}

	if ((L1.sets)[index].dirty[temp] == 1 && (L1.sets)[index].valid[temp] == 1) {
				int64_t concate = ((L1.sets)[index].tag[temp] << (c - b - s)) + index;
				int64_t Tag = (concate >> (C - S - b)) & ((1 << (64 - C + S)) - 1);
				int64_t Index = concate & ((1 << (C - S - b)) - 1);
				evict_to_L2(1, Tag, Index, stats);
	}



	int64_t min = 9999999999;
	for (int64_t i = 0; i < L1_ways; i++) {
		if ((L1.sets)[index].counter[i] < min && (L1.sets)[index].valid[i] == 1) {
			min = (L1.sets)[index].counter[i];
		}
	}
	(L1.sets)[index].valid[temp] = 1;
	(L1.sets)[index].tag[temp] = tag;
	(L1.sets)[index].dirty[temp] = isDirty;
	(L1.sets)[index].counter[temp] = min - 1; // MRU

}



int64_t L1_hit() {
    	for (int64_t i = 0; i < L1_ways; i++) {
        	if ((L1.sets)[L1_index].tag[i] == L1_tag && (L1.sets)[L1_index].valid[i] == 1) {
            		return i;
        	}
    	}
	return -1;
}

int64_t vic_hit() {
	for (int64_t i = 0; i < v; i++) {
		if (vic.tag[i] == vic_tag && vic.valid[i] == 1) {
			return i;
		}
	}
	return -1;
}

int64_t L2_hit(struct cache_stats_t *stats) {
    for (int64_t i = 0; i < L2_ways; i++) {
    		if ((L2.sets)[L2_index].tag[i] == L2_tag && (L2.sets)[L2_index].valid[i] == 1) {
						if ((L2.sets)[L2_index].prefetch[i] == 1) {
								stats->num_useful_prefetches++;
								(L2.sets)[L2_index].prefetch[i] = 0;
						}
            return i;
        }
		}
		return -1;
}

void prefetch(int64_t tag, int64_t index, struct cache_stats_t *stats) { // LRU

	for (int64_t i = 0; i < L2_ways; i++) {
		if ((L2.sets)[index].tag[i] == tag && (L2.sets)[index].valid[i] == 1) {
				return;
		}
	}
	stats->num_prefetches++;
	stats->num_bytes_transferred++; // prefetch

	for (int64_t i = 0; i < L2_ways; i++) {
		if ((L2.sets)[index].valid[i] == 0) { // find empty space

			int64_t max = -9999999999;
			for (int64_t j = 0; j < L2_ways; j++) {
				if ((L2.sets)[index].counter[j] > max && (L2.sets)[index].valid[j] == 1) {
					max = (L2.sets)[index].counter[j];
				}
			}
			(L2.sets)[index].valid[i] = 1;
	  	(L2.sets)[index].tag[i] = tag;
			(L2.sets)[index].dirty[i] = 0;
			(L2.sets)[index].prefetch[i] = 1;
			(L2.sets)[index].counter[i] = max + 1; // LRU

			return;
		}
	}

	// full
	int64_t max = -9999999999;
	int64_t temp = -1;
	for (int64_t i = 0; i < L2_ways; i++) {
		if ((L2.sets)[index].counter[i] > max && (L2.sets)[index].valid[i] == 1) {
			max = (L2.sets)[index].counter[i];
			temp = i;
		}
	}

	if ((L2.sets)[index].dirty[temp] == 1) {
		stats->num_write_backs++;
		stats->num_bytes_transferred++; // write back
	}

	(L2.sets)[index].tag[temp] = tag;
	(L2.sets)[index].valid[temp] = 1;
	(L2.sets)[index].dirty[temp] = 0;
	(L2.sets)[index].prefetch[temp] = 1;
	(L2.sets)[index].counter[temp] = max + 1; // LRU
}

void install_to_L1(int64_t isDirty, int64_t tag, int64_t index, struct cache_stats_t *stats) { // MRU

	for (int64_t i = 0; i < L1_ways; i++) { // find empty space
		if ((L1.sets)[index].valid[i] == 0) {

			int64_t min = 9999999999;
			for (int64_t j = 0; j < L1_ways; j++) {
				if ((L1.sets)[index].counter[j] < min && (L1.sets)[index].valid[j] == 1) {
					min = (L1.sets)[index].counter[j];
				}
			}
			(L1.sets)[index].valid[i] = 1;
	  		(L1.sets)[index].tag[i] = tag;
			(L1.sets)[index].dirty[i] = isDirty;
			(L1.sets)[index].counter[i] = min - 1; // MRU

			return;
		}
	}

	// full
	int64_t max = -9999999999;
	int64_t temp = -1;
	for (int64_t i = 0; i < L1_ways; i++) {
		if ((L1.sets)[index].counter[i] > max && (L1.sets)[index].valid[i] == 1) {
			max = (L1.sets)[index].counter[i];
			temp = i;
		}
	}

	int64_t Dirty = (L1.sets)[index].dirty[temp];
	int64_t Tag = ((L1.sets)[index].tag[temp] << (c - s - b)) + index;
	evict_to_vic(Dirty, Tag, stats);

	int64_t min = 9999999999;
	for (int64_t i = 0; i < L1_ways; i++) {
		if ((L1.sets)[index].counter[i] < min && (L1.sets)[index].valid[i] == 1) {
			min = (L1.sets)[index].counter[i];
		}
	}
	(L1.sets)[index].valid[temp] = 1;
	(L1.sets)[index].tag[temp] = tag;
	(L1.sets)[index].dirty[temp] = isDirty;
	(L1.sets)[index].counter[temp] = min - 1; // MRU

}

void evict_to_vic(int64_t isDirty, int64_t tag, struct cache_stats_t *stats) { // FIFO

	for (int64_t i = 0; i < v; i++) {
			if (vic.valid[i] == 0) { // find empty space

					int64_t min = 9999999999;
					for (int64_t j = 0; j < v; j++) {
							if (vic.counter[j] < min && vic.valid[j] == 1) {
									min = vic.counter[j];
							}
					}
					vic.counter[i] = min - 1;
					vic.valid[i] = 1;
					vic.dirty[i] = isDirty;
					vic.tag[i] = tag;
			}
	}

	// full

	int64_t max = -9999999999;
	int64_t temp = -1;
	for (int64_t i = 0; i < v; i++) {
			if (vic.counter[i] > max && vic.valid[i] == 1) {
					max = vic.counter[i];
					temp = i;
			}
	}

	if (vic.dirty[temp] == 1) {
		int64_t Tag = (vic.tag[temp] >> (C - S - b)) & ((1 << (64 - C + S)) - 1);
		int64_t Index = vic.tag[temp] & ((1 << (C - S - b)) - 1);
		evict_to_L2(1, Tag, Index, stats);
	}

  int64_t min = 9999999999;
	for (int64_t i = 0; i < v; i++) {
			if (vic.counter[i] < min && vic.valid[i] == 1) {
					min = vic.counter[i];
			}
	}

	vic.dirty[temp] = isDirty;
	vic.valid[temp] = 1;
	vic.tag[temp] = tag;
	vic.counter[temp] = min - 1;
}

void install_to_L2(int64_t isDirty, int64_t tag, int64_t index, struct cache_stats_t *stats) { // MRU
	stats->num_bytes_transferred++; // miss repair
	for (int64_t i = 0; i < L2_ways; i++) {
		if ((L2.sets)[index].valid[i] == 0) { // find empty space

			int64_t min = 9999999999;
			for (int64_t j = 0; j < L2_ways; j++) {
				if ((L2.sets)[index].counter[j] < min && (L2.sets)[index].valid[j] == 1) {
					min = (L2.sets)[index].counter[j];
				}
			}
			(L2.sets)[index].valid[i] = 1;
	  		(L2.sets)[index].tag[i] = tag;
			(L2.sets)[index].dirty[i] = isDirty;
			(L2.sets)[index].counter[i] = min - 1; // MRU
			(L2.sets)[index].prefetch[i] = 0;

			return;
		}
	}

	// full
	int64_t max = -9999999999;
	int64_t temp = -1;
	for (int64_t i = 0; i < L2_ways; i++) {
		if ((L2.sets)[index].counter[i] > max && (L2.sets)[index].valid[i] == 1) {
			max = (L2.sets)[index].counter[i];
			temp = i;
		}
	}

	if ((L2.sets)[index].dirty[temp] == 1) {
		stats->num_write_backs++;
		stats->num_bytes_transferred++;
	}

	int64_t min = 9999999999;
	for (int64_t i = 0; i < L2_ways; i++) {
		if ((L2.sets)[index].counter[i] < min && (L2.sets)[index].valid[i] == 1) {
			min = (L2.sets)[index].counter[i];
		}
	}
	(L2.sets)[index].tag[temp] = tag;
	(L2.sets)[index].valid[temp] = 1;
	(L2.sets)[index].dirty[temp] = isDirty;
	(L2.sets)[index].counter[temp] = min - 1;
		(L2.sets)[index].prefetch[temp] = 0;
}

void evict_to_L2(int64_t isDirty, int64_t tag, int64_t index, struct cache_stats_t *stats) { // LRU
	for (int64_t i = 0; i < L2_ways; i++) {
		if ((L2.sets)[index].valid[i] == 1 && (L2.sets)[index].tag[i] == tag) {
				(L2.sets)[index].dirty[i] = 1;
				return;
		}
	}

	for (int64_t i = 0; i < L2_ways; i++) { // find empty space
		if ((L2.sets)[index].valid[i] == 0) {

			int64_t max = -9999999999;
			for (int64_t j = 0; j < L2_ways; j++) {
				if ((L2.sets)[index].counter[j] > max && (L2.sets)[index].valid[j] == 1) {
						max = (L2.sets)[index].counter[j];
				}
			}
			(L2.sets)[index].valid[i] = 1;
	  		(L2.sets)[index].tag[i] = tag;
			(L2.sets)[index].dirty[i] = isDirty;
			(L2.sets)[index].counter[i] = max + 1; // LRU
			(L2.sets)[index].prefetch[i] = 0;

			return;
		}
	}

	// full
	int64_t max = -9999999999;
	int64_t temp = -1;
	for (int64_t i = 0; i < L2_ways; i++) {
		if ((L2.sets)[index].counter[i] > max && (L2.sets)[index].valid[i] == 1) {
			max = (L2.sets)[index].counter[i];
			temp = i;
		}
	}

	if ((L2.sets)[index].dirty[temp] == 1 && (L2.sets)[index].valid[temp] == 1) {
		stats->num_write_backs++;
		stats->num_bytes_transferred++;
	}

	(L2.sets)[index].valid[temp] = 1;
	(L2.sets)[index].tag[temp] = tag;
	(L2.sets)[index].dirty[temp] = isDirty;
	(L2.sets)[index].counter[temp] = max + 1; // LRU
	(L2.sets)[index].prefetch[temp] = 0;
}

/** @brief Function to free any allocated memory and finalize statistics
 *
 *  @param stats pointer to the cache statistics structure
 *
 */
void cache_cleanup(struct cache_stats_t *stats)
{

  uint64_t bytes = uint64_t(1 << b);
  stats->num_bytes_transferred *= bytes;

  stats->miss_rate_l1 = double(stats->num_misses_l1) / double(stats->num_accesses);

  if (v == 0) {
      stats->miss_rate_vc = 1;
      stats->miss_rate_l2 = double(stats->num_misses_l2) / double(stats->num_misses_l1);
      stats->avg_access_time = stats->hit_time_l1 + stats->miss_rate_l1 * (stats->hit_time_l2 + stats->miss_rate_l2 * stats->hit_time_mem);
  } else {
      stats->miss_rate_vc = double(stats->num_misses_vc) / double(stats->num_misses_l1);
      stats->miss_rate_l2 = double(stats->num_misses_l2) / double(stats->num_misses_vc);
      stats->avg_access_time = stats->hit_time_l1 + stats->miss_rate_l1 * stats->miss_rate_vc * (stats->hit_time_l2 + stats->miss_rate_l2 * stats->hit_time_mem);
  }

  for (int64_t i = 0; i < L1_sets; i++) {
      delete[] (L1.sets)[i].counter;
      delete[] (L1.sets)[i].tag;
      delete[] (L1.sets)[i].valid;
      delete[] (L1.sets)[i].dirty;
  }

  delete[] L1.sets;

  for (int64_t i = 0; i < L2_sets; i++) {
      delete[] (L2.sets)[i].counter;
      delete[] (L2.sets)[i].tag;
      delete[] (L2.sets)[i].valid;
      delete[] (L2.sets)[i].dirty;
      delete[] (L2.sets)[i].prefetch;
  }

  delete[] L2.sets;

  delete[] vic.counter;
  delete[] vic.tag;
  delete[] vic.valid;
  delete[] vic.dirty;
}
