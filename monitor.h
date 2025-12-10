#ifndef MONITOR_H
#define MONITOR_H

typedef struct {
    int pid;
    char name[256];
    char state;
    unsigned long memory_kb;
    unsigned long long old_utime;
    unsigned long long old_stime;
    double cpu_usage;
    int active;
} ProcessInfo;

typedef struct {
    double cpu_percent;
    double mem_percent;
    unsigned long total_mem_kb;
    unsigned long used_mem_kb;
} GlobalStats;

// API
void update_stats();
int get_process_count();
ProcessInfo* get_process_list();
GlobalStats* get_global_stats();

void get_system_cpu(unsigned long long *total, unsigned long long *idle);
void get_system_mem(GlobalStats *stats);
int get_proc_details(int pid, ProcessInfo *p);

#endif
