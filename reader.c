#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "monitor.h"

void get_system_cpu(unsigned long long *total, unsigned long long *idle_val) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) { *total = 0; *idle_val = 0; return; }

    char line[512];
    if (!fgets(line, sizeof(line), fp)) { fclose(fp); *total = 0; *idle_val = 0; return; }
    fclose(fp);

    unsigned long long user=0,nice=0,system=0,idle=0,iowait=0,irq=0,softirq=0,steal=0;
    sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &user,&nice,&system,&idle,&iowait,&irq,&softirq,&steal);

    *idle_val = idle + iowait;
    *total = user + nice + system + idle + iowait + irq + softirq + steal;
}

void get_system_mem(GlobalStats *stats) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) { stats->total_mem_kb=0; stats->used_mem_kb=0; stats->mem_percent=0; return; }

    char line[256];
    unsigned long total=0,avail=0;

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "MemTotal: %lu kB", &total);
        sscanf(line, "MemAvailable: %lu kB", &avail);
    }
    fclose(fp);

    stats->total_mem_kb = total;
    stats->used_mem_kb  = (total > avail) ? (total - avail) : 0;
    stats->mem_percent  = (total > 0) ? ((double)stats->used_mem_kb / total * 100.0) : 0.0;
}

int get_proc_details(int pid, ProcessInfo *p) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    char buf[2048];
    if (!fgets(buf, sizeof(buf), fp)) { fclose(fp); return 0; }
    fclose(fp);

    char *l = strchr(buf, '(');
    char *r = strrchr(buf, ')');
    if (!l || !r || r < l) return 0;

    size_t len = r - l - 1;
    if (len > 255) len = 255;
    memcpy(p->name, l+1, len);
    p->name[len] = '\0';

    char *rest = r + 2;
    char state = 'R';
    unsigned long utime=0, stime=0;

    // FIXED SCAN LINE (NO WARNINGS)
    sscanf(rest,
        "%c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu %lu",
        &state, &utime, &stime);

    p->pid = pid;
    p->state = state;
    p->old_utime = utime;
    p->old_stime = stime;

    // memory
    snprintf(path, sizeof(path), "/proc/%d/statm", pid);
    fp = fopen(path, "r");
    if (fp) {
        unsigned long size,res;
        if (fscanf(fp, "%lu %lu", &size, &res) == 2) {
            long pg = sysconf(_SC_PAGESIZE);
            if (pg <= 0) pg = 4096;
            p->memory_kb = (res * pg) / 1024;
        }
        fclose(fp);
    }

    p->cpu_usage = 0.0;
    p->active = 0;
    return 1;
}
