#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include "monitor.h"

#define MAX_PROCS 4096

static ProcessInfo proc_list[MAX_PROCS];
static int proc_count = 0;
static GlobalStats gstats = {0};

static unsigned long long prev_total = 0;
static unsigned long long prev_idle = 0;

ProcessInfo* get_process_list() { return proc_list; }
int get_process_count() { return proc_count; }
GlobalStats* get_global_stats() { return &gstats; }

static ProcessInfo* find_proc(int pid) {
    for (int i = 0; i < proc_count; ++i) if (proc_list[i].pid == pid) return &proc_list[i];
    return NULL;
}

extern void get_system_cpu(unsigned long long *total, unsigned long long *idle);
extern void get_system_mem(GlobalStats *stats);
extern int get_proc_details(int pid, ProcessInfo *p);

void update_stats() {
    DIR *d = opendir("/proc");
    if (!d) return;

    unsigned long long total = 0, idle = 0;
    get_system_cpu(&total, &idle);

    unsigned long long tdelta = (prev_total ? (total - prev_total) : 0ULL);
    unsigned long long idelta = (prev_idle ? (idle - prev_idle) : 0ULL);

    if (tdelta == 0) {
        gstats.cpu_percent = 0.0;
    } else {
        double used = (double)(tdelta - idelta);
        gstats.cpu_percent = (used / (double)tdelta) * 100.0;
        if (gstats.cpu_percent < 0.0) gstats.cpu_percent = 0.0;
        if (gstats.cpu_percent > 100.0) gstats.cpu_percent = 100.0;
    }

    prev_total = total;
    prev_idle = idle;

    get_system_mem(&gstats);

    // mark inactive
    for (int i = 0; i < proc_count; ++i) proc_list[i].active = 0;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (!isdigit((unsigned char)ent->d_name[0])) continue;
        int pid = atoi(ent->d_name);

        ProcessInfo tmp;
        memset(&tmp, 0, sizeof(tmp));
        if (!get_proc_details(pid, &tmp)) continue;

        ProcessInfo *ex = find_proc(pid);
        unsigned long long curr_ticks = tmp.old_utime + tmp.old_stime;

        if (ex) {
            unsigned long long prev_ticks = ex->old_utime + ex->old_stime;
            long long delta = (long long)curr_ticks - (long long)prev_ticks;
            if (delta < 0) delta = 0;

            double cpu = (tdelta > 0) ? ((double)delta / (double)tdelta) * 100.0 : 0.0;
            if (cpu < 0.0) cpu = 0.0;
            if (cpu > 100.0) cpu = 100.0;

            ex->cpu_usage = cpu;
            ex->old_utime = tmp.old_utime;
            ex->old_stime = tmp.old_stime;
            ex->memory_kb = tmp.memory_kb;
            ex->state = tmp.state;
            ex->active = 1;
        } else {
            if (proc_count < MAX_PROCS) {
                proc_list[proc_count] = tmp;
                proc_list[proc_count].cpu_usage = 0.0; // first cycle no delta
                proc_list[proc_count].active = 1;
                proc_count++;
            }
        }
    }
    closedir(d);

    // compact list, drop inactive
    int w = 0;
    for (int i = 0; i < proc_count; ++i) {
        if (proc_list[i].active) {
            if (w != i) proc_list[w] = proc_list[i];
            ++w;
        }
    }
    proc_count = w;
}
