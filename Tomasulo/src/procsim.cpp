#include <cstdio>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <list>
#include <iostream>

#include "procsim.hpp"

using namespace std;

uint64_t ID = 0; // instruction ID start with 0
uint64_t f, k0, k1, k2, r, g, c, sets, G;
int64_t misprediction_ID = -1;
bool misprediction = false;
int32_t GHR = 0;

uint64_t sum_sq;


Scheduling_Queue sq;
Register_File rf;
inst_t inst;
list<inst_t> dq;
list<int32_t> dq_misprediction;
Score_Board sb;
Result result;
list<inst_t> fetch_buffer;
Cache cache;
BTB btb;






int check_sq_full();
void sort();

/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 *
 * param config Pointer to the run configuration structure
 */
void setup_proc(proc_conf_t *config)
{
    f = config->f;
    k0 = config->k0;
    k1 = config->k1;
    k2 = config->k2;
    r = config->r;
    g = config->g;
    c = config->c;
    sets = 1 << (c - 6);
    G =  1 << g;

    btb.smith = new int32_t[G];

    for (uint64_t i = 0; i < G; i++) {
        btb.smith[i] = 1; // start with 01
    }

    cache.tag = new uint64_t[sets];
    cache.valid = new uint64_t[sets];

    for (uint64_t i = 0; i < sets; i++) {
        cache.tag[i] = 0;
        cache.valid[i] = 0;
    }

    result.fetch = new uint64_t[2000000];
    result.dispatch = new uint64_t[2000000];
    result.schedule = new uint64_t[2000000];
    result.execute = new uint64_t[2000000];
    result.update = new uint64_t[2000000];
    result.opcode = new int32_t[2000000];
    result.actual_taken = new int32_t[2000000];
    result.address = new uint64_t[2000000];

    for (int i = 0; i < 2000000; i++) {
        result.fetch[i] = 0;
        result.dispatch[i] = 0;
        result.schedule[i] = 0;
        result.execute[i] = 0;
        result.update[i] = 0;
        result.opcode[i] = 0;
        result.actual_taken[i] = 100; // no meaning
        result.address[i] = 0;
    }

    rf.ready = new int32_t[32];
    rf.tag = new int32_t[32];

    for (int i = 0; i < 32; i++) {
        rf.ready[i] = 1;
        rf.tag[i] = -1;
    }

    sq.op = new int32_t[r * (k0 + k1 + k2)];
    sq.dest_reg = new int32_t[r * (k0 + k1 + k2)];
    sq.dest_tag = new int32_t[r * (k0 + k1 + k2)];
    sq.src1_ready = new int32_t[r * (k0 + k1 + k2)];
    sq.src1_tag = new int32_t[r * (k0 + k1 + k2)];
    sq.src2_ready = new int32_t[r * (k0 + k1 + k2)];
    sq.src2_tag = new int32_t[r * (k0 + k1 + k2)];
    sq.execution_time = new int32_t[r * (k0 + k1 + k2)];
    sq.address = new int64_t[r * (k0 + k1 + k2)];

    for (uint64_t i = 0; i < r * (k0 + k1 + k2); i++) {
        sq.op[i] = 0;
        sq.dest_reg[i] = 0;
        sq.dest_tag[i] = -1;
        sq.src1_ready[i] = 0;
        sq.src1_tag[i] = -1;
        sq.src2_ready[i] = 0;
        sq.src2_tag[i] = -1;
        sq.execution_time[i] = -3000000;
        sq.address[i] = -1;
    }

    sb.k0_busy = new int32_t[k0];
    sb.k1_busy = new int32_t[k1];
    sb.k2_busy = new int32_t[k2];
    sb.address = new int64_t[k2];

    for (uint64_t i = 0; i < k0; i++) {
        sb.k0_busy[i] = 0;
    }

    for (uint64_t i = 0; i < k1; i++) {
        sb.k1_busy[i] = 0;
    }

    for (uint64_t i = 0; i < k2; i++) {
        sb.k2_busy[i] = 0;
        sb.address[i] = 0;
    }
}




int check_sq_full() { // return -1 if full
    for (uint64_t i = 0; i < r * (k0 + k1 + k2); i++) {
        if (sq.dest_tag[i] == -1 && sq.execution_time[i] == -3000000) {
            return int(i);
        }
    }
    return -1;
}

void sort() {
    for (uint64_t i = 0; i < r * (k0 + k1 + k2) - 1; i++) {
        int32_t min = sq.dest_tag[i];
        int index = i;
        for (uint64_t j = i + 1; j < r * (k0 + k1 + k2); j++) {
            if (sq.dest_tag[j] < min) {
                min = sq.dest_tag[j];
                index = j;
            }
        }

        int32_t temp1 = sq.op[index];
        int32_t temp2 = sq.dest_reg[index];
        int32_t temp3 = sq.dest_tag[index];
        int32_t temp4 = sq.src1_ready[index];
        int32_t temp5 = sq.src1_tag[index];
        int32_t temp6 = sq.src2_ready[index];
        int32_t temp7 = sq.src2_tag[index];
        int32_t temp8 = sq.execution_time[index];
        int64_t temp9 = sq.address[index];

        sq.op[index] = sq.op[i];
        sq.dest_reg[index] = sq.dest_reg[i];
        sq.dest_tag[index] = sq.dest_tag[i];
        sq.src1_ready[index] = sq.src1_ready[i];
        sq.src1_tag[index] = sq.src1_tag[i];
        sq.src2_ready[index] = sq.src2_ready[i];
        sq.src2_tag[index] = sq.src2_tag[i];
        sq.execution_time[index] = sq.execution_time[i];
        sq.address[index] = sq.address[i];

        sq.op[i] = temp1;
        sq.dest_reg[i] = temp2;
        sq.dest_tag[i] = temp3;
        sq.src1_ready[i] = temp4;
        sq.src1_tag[i] = temp5;
        sq.src2_ready[i] = temp6;
        sq.src2_tag[i] = temp7;
        sq.execution_time[i] = temp8;
        sq.address[i] = temp9;
    }
}

/**
 * Subroutine that simulates the processor. The processor should fetch instructions as
 * appropriate, until all instructions have executed
 *
 * param p_stats Pointer to the statistics structure
 */
void run_proc(proc_stats_t* p_stats)
{
    bool ret = true;
    bool continue_execution = true;

    while (ret || continue_execution) {

        // execute
        for (uint64_t i = 0; i < r * (k0 + k1 + k2); i++) {
            if (sq.dest_tag[i] != -1) {
                if (sq.execution_time[i] != -3000000) { // already fired
                    sq.execution_time[i]--;
                }
                if (sq.execution_time[i] == 0) {
                    result.update[sq.dest_tag[i]] = p_stats->cycle_count;
                    if (sq.op[i] == 4 || sq.op[i] == 5) {
                        for (uint64_t j = 0; j < k2; j++) {
                            if (sb.k2_busy[j] == 1 && sb.address[j] == sq.address[i]) {
                                sb.k2_busy[j] = 0;
                                sb.address[j] = 0;

                                break;
                            }
                        }
                    }
                }
            }
        }

        for (uint64_t i = 0; i < k0; i++) {
            sb.k0_busy[i] = 0;
        }

        for (uint64_t i = 0; i < k1; i++) {
            sb.k1_busy[i] = 0;
        }

        sort();

        // fire
        for (uint64_t i = 0; i < r * (k0 + k1 + k2); i++) {
            if (sq.dest_tag[i] != -1) { // RS is not empty
                if (sq.src1_ready[i] == 1 && sq.src2_ready[i] == 1 && sq.execution_time[i] == -3000000) { // both sources are ready

                    if (sq.op[i] == 2 || sq.op[i] == 6) { // use k0
                        for (uint64_t j = 0; j < k0; j++) {
                            if (sb.k0_busy[j] == 0) {
                                result.execute[sq.dest_tag[i]] = p_stats->cycle_count;
                                sb.k0_busy[j] = 1;
                                sq.execution_time[i] = 1; // add/branch is 1 cycle
                                break;
                            }
                        }
                    } else if (sq.op[i] == 3) { // use k1
                        for (uint64_t j = 0; j < k1; j++) {
                            if (sb.k1_busy[j] == 0) {
                                result.execute[sq.dest_tag[i]] = p_stats->cycle_count;
                                sb.k1_busy[j] = 1;
                                sq.execution_time[i] = 3; // mul is 3 cycles
                                break;
                            }
                        }
                    } else if (sq.op[i] == 4) { // lw, use k2


                        uint64_t TAG = (sq.address[i] >> c);
                        uint64_t INDEX = (sq.address[i] >> 6) & ((1 << (c - 6)) - 1);

                        bool flag = false;
                        for (uint64_t j = 0; j < k2; j++) {
                            uint64_t index = (sb.address[j] >> 6) & ((1 << (c - 6)) - 1);
                            if (sb.k2_busy[j] == 1 && INDEX == index) {
                                flag = true;
                                break;
                            }
                        }

                        if (flag == true) {
                            continue;
                        }

                        bool flag2 = false;
                        for (uint64_t k = 0; k < i; k++) {
                            if (sq.address[k] == sq.address[i] && sq.op[k] == 5) {
                                flag2 = true;
                                break;
                            }
                        }

                        if (flag2 == true) {
                            continue;
                        }

                        for (uint64_t j = 0; j < k2; j++) {
                            if (sb.k2_busy[j] == 0) {
                                bool cache_miss;
                                if (cache.valid[INDEX] == 1 && cache.tag[INDEX] == TAG) { // cache hit
                                    cache_miss = false;
                                } else { // cache miss
                                    cache_miss = true;
                                    p_stats->cache_misses++;
                                    cache.valid[INDEX] = 1;
                                    cache.tag[INDEX] = TAG;
                                }

                                result.execute[sq.dest_tag[i]] = p_stats->cycle_count;
                                sb.k2_busy[j] = 1;
                                sb.address[j] = sq.address[i]; // fire

                                if (cache_miss) {
                                    sq.execution_time[i] = 10; // cache miss is 10 cycles
                                } else {
                                    sq.execution_time[i] = 1;
                                }
                                break;
                            }
                        }


                    } else if (sq.op[i] == 5) { // sw, use k2



                      uint64_t TAG = (sq.address[i] >> c);
                      uint64_t INDEX = (sq.address[i] >> 6) & ((1 << (c - 6)) - 1);

                      bool flag = false;
                      for (uint64_t j = 0; j < k2; j++) {
                          uint64_t index = (sb.address[j] >> 6) & ((1 << (c - 6)) - 1);
                          if (sb.k2_busy[j] == 1 && INDEX == index) {
                              flag = true;
                              break;
                          }
                      }

                      if (flag == true) {
                          continue;
                      }

                      bool flag2 = false;
                      for (uint64_t k = 0; k < i; k++) {
                          if (sq.address[k] == sq.address[i] && (sq.op[k] == 5 || sq.op[k] == 4)) {
                              flag2 = true;
                              break;
                          }
                      }

                      if (flag2 == true) {
                          continue;
                      }

                      for (uint64_t j = 0; j < k2; j++) {
                          if (sb.k2_busy[j] == 0) {
                              bool cache_miss;
                              if (cache.valid[INDEX] == 1 && cache.tag[INDEX] == TAG) { // cache hit
                                  cache_miss = false;
                              } else { // cache miss
                                  cache_miss = true;
                                  p_stats->cache_misses++;
                                  cache.valid[INDEX] = 1;
                                  cache.tag[INDEX] = TAG;
                              }

                              result.execute[sq.dest_tag[i]] = p_stats->cycle_count;
                              sb.k2_busy[j] = 1;
                              sb.address[j] = sq.address[i]; // fire

                              if (cache_miss) {
                                  sq.execution_time[i] = 10; // cache miss is 10 cycles
                              } else {
                                  sq.execution_time[i] = 1;
                              }
                              break;
                          }
                      }




                    }
                }
            }
        }



        // disptach to schedule
        if (misprediction == false) { // if predict correctly
            while (!dq.empty()) {
                int return_value = check_sq_full();
                if (return_value == -1 || misprediction == true) {
                    break;
                } else {
                    inst_t temp = dq.front();
                    dq.pop_front();
                    int32_t TEMP = dq_misprediction.front();
                    dq_misprediction.pop_front();

                    if (temp.opcode == 1) {
                        continue;
                    }

                    result.schedule[temp.id] = p_stats->cycle_count;

                    sq.op[return_value] = temp.opcode;
                    sq.dest_reg[return_value] = temp.dest_reg;
                    sq.execution_time[return_value] = -3000000;

                    if (temp.src_reg[0] == -1) {
                        sq.src1_ready[return_value] = 1;
                    } else {
                        if (rf.ready[temp.src_reg[0]] == 1) {
                            sq.src1_ready[return_value] = 1;
                        } else {
                            sq.src1_ready[return_value] = 0;
                            sq.src1_tag[return_value] = rf.tag[temp.src_reg[0]];
                        }
                    }

                    if (temp.src_reg[1] == -1) {
                        sq.src2_ready[return_value] = 1;
                    } else {
                        if (rf.ready[temp.src_reg[1]] == 1) {
                            sq.src2_ready[return_value] = 1;
                        } else {
                            sq.src2_ready[return_value] = 0;
                            sq.src2_tag[return_value] = rf.tag[temp.src_reg[1]];
                        }
                    }

                    sq.dest_tag[return_value] = temp.id;
                    if (temp.dest_reg != -1) {
                        rf.tag[temp.dest_reg] = temp.id;
                        rf.ready[temp.dest_reg] = 0;
                    }

                    sq.address[return_value] = temp.ld_st_addr;
                    if (temp.opcode == 6) {
                        if (TEMP == 1) { // mispredict
                            misprediction = true;
                            misprediction_ID = temp.id;
                            break;
                        }
                    }
                }
            }
        }




        for (uint64_t i = 0; i < r * (k0 + k1 + k2); i++) {
            if (sq.dest_tag[i] != -1 && sq.execution_time[i] == 0) {

                if (misprediction_ID == int64_t(sq.dest_tag[i]) && misprediction == true) {
                    misprediction = false;
                    misprediction_ID = -1;
                }

                sq.address[i] = -1;

                if (sq.dest_reg[i] == -1) {
                    continue;
                }

                if (sq.dest_tag[i] == rf.tag[sq.dest_reg[i]]) {
                    rf.ready[sq.dest_reg[i]] = 1;
                }
                for (uint64_t j = 0; j < r * (k0 + k1 + k2); j++) {
                    if (!sq.src1_ready[j] && sq.src1_tag[j] == sq.dest_tag[i]) {
                        sq.src1_ready[j] = 1;
                    }
                    if (!sq.src2_ready[j] && sq.src2_tag[j] == sq.dest_tag[i]) {
                        sq.src2_ready[j] = 1;
                    }
                }

            }

            if (sq.dest_tag[i] != -1 && sq.execution_time[i] == -1) {


              if (sq.op[i] == 6) {

                  if (result.actual_taken[sq.dest_tag[i]] == 1) { // actually taken
                      if (btb.smith[((result.address[sq.dest_tag[i]] >> 2) % (1 << g)) ^ GHR] != 3) {
                          btb.smith[((result.address[sq.dest_tag[i]] >> 2) % (1 << g)) ^ GHR]++;
                      }
                  } else { // actually not taken
                      if (btb.smith[((result.address[sq.dest_tag[i]] >> 2) % (1 << g)) ^ GHR] != 0) {
                          btb.smith[((result.address[sq.dest_tag[i]] >> 2) % (1 << g)) ^ GHR]--;
                      }
                  }

                  int32_t NNN = (1 << int32_t(g)) - 1;
                  if (result.actual_taken[sq.dest_tag[i]] == 1) { // actually taken
                      GHR = ((GHR << 1) + 1) & NNN;
                  } else { // actually not taken
                      GHR = (GHR << 1) & NNN;
                  }
              }

                sq.dest_tag[i] = -1;
                sq.execution_time[i] = -3000000;
            }
        }


        bool FLAGG = true;
        for (uint64_t i = 0; i < r * (k0 + k1 + k2); i++) {
            if (sq.dest_tag[i] != -1 || sq.execution_time[i] != -3000000) { // not retired
                FLAGG = false;
            }
        }

        if (FLAGG == true) { // all retired
            continue_execution = false;
        } else {
            continue_execution = true;
        }

        if (continue_execution == false && ret == false) {
            break;
        }








        // fetch_buffer
        while (!fetch_buffer.empty()) {
            inst_t temp = fetch_buffer.front();
            result.dispatch[temp.id] = p_stats->cycle_count;
            fetch_buffer.pop_front();
            dq.push_back(temp);

            if (temp.opcode == 6) {
                bool taken;
                if (btb.smith[((temp.inst_addr >> 2) % (1 << g)) ^ GHR] >= 2) {
                    taken = true;
                } else {
                    taken = false;
                }
                if (taken == temp.br_taken) {
                    dq_misprediction.push_back(0);
                    p_stats->correctly_predicted++;
                } else {
                    dq_misprediction.push_back(1);
                }
            } else {
                dq_misprediction.push_back(100); // no meaning
            }
        }

        // fetch
        for (uint64_t i = 0; i < f; i++) {
            ret = read_instruction(&inst);
            if (ret == false) {
                break;
            } else {

                inst.id = ID;
                result.fetch[inst.id] = p_stats->cycle_count;
                result.opcode[inst.id] = inst.opcode;
                result.address[inst.id] = inst.inst_addr;

                if (inst.opcode == 6) {
                    if (inst.br_taken == true) {
                        result.actual_taken[inst.id] = 1;
                    } else {
                        result.actual_taken[inst.id] = 0;
                    }
                }

                ID++;
                if (inst.opcode != 1) {
                    p_stats->instructions_retired++;
                }

                if (inst.opcode == 6) {
                    p_stats->branch_instructions++;
                }
                if (inst.opcode == 4) {
                    p_stats->load_instructions++;
                }
                if (inst.opcode == 5) {
                    p_stats->store_instructions++;
                }

                fetch_buffer.push_back(inst);

            }
        }

        sum_sq += dq.size();
        if (p_stats->max_disp_queue_size < dq.size()) {
            p_stats->max_disp_queue_size = dq.size();
        }

        p_stats->cycle_count++;
    }

    p_stats->cycle_count--;
}



/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC, average fire rate etc.
 *
 * param p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats)
{
    cout << "FETCH	DISP	SCHED	EXEC	SUPDATE" << endl;
    for (int i = 0; i < 2000000; i++) {
        if (result.opcode[i] != 1) {
            cout << result.fetch[i] << " " << result.dispatch[i] << " " << result.schedule[i] << " " << result.execute[i] << " " << result.update[i] << endl;
        }
    }
    p_stats->cache_miss_rate = ((double) p_stats->cache_misses) / (p_stats->load_instructions + p_stats->store_instructions);
    p_stats->average_instructions_retired = ((double) p_stats->instructions_retired) / p_stats->cycle_count;
    p_stats->branch_prediction_accuracy = ((double) p_stats->correctly_predicted) / p_stats->branch_instructions;
    p_stats->average_disp_queue_size = ((double) sum_sq) / p_stats->cycle_count;

    delete[] cache.tag;
    delete[] cache.valid;

    delete[] btb.smith;

    delete[] result.fetch;
    delete[] result.dispatch;
    delete[] result.schedule;
    delete[] result.execute;
    delete[] result.update;
    delete[] result.opcode;
    delete[] result.actual_taken;
    delete[] result.address;

    delete[] sq.op;
    delete[] sq.dest_reg;
    delete[] sq.dest_tag;
    delete[] sq.src1_ready;
    delete[] sq.src1_tag;
    delete[] sq.src2_ready;
    delete[] sq.src2_tag;
    delete[] sq.execution_time;
    delete[] sq.address;

    delete[] rf.ready;
    delete[] rf.tag;

    delete[] sb.k0_busy;
    delete[] sb.k1_busy;
    delete[] sb.k2_busy;
    delete[] sb.address;

    fetch_buffer.clear();
    dq.clear();
    dq_misprediction.clear();
}
