#include "monitor.h"

int compare_mem(const void *a, const void *b) {
    ProcessInfo *procA = (ProcessInfo *)a;
    ProcessInfo *procB = (ProcessInfo *)b;
    return (procB->rss - procA->rss);
}

void sort_processes_by_mem(ProcessInfo *procs, int count) {
    qsort(procs, count, sizeof(ProcessInfo), compare_mem);
}

void calculate_cpu_usage(CpuStats *stats) {
    unsigned long long prev_total = stats->prev.user + stats->prev.nice + stats->prev.system + stats->prev.idle +
                                    stats->prev.iowait + stats->prev.irq + stats->prev.softirq + stats->prev.steal;
    unsigned long long curr_total = stats->curr.user + stats->curr.nice + stats->curr.system + stats->curr.idle +
                                    stats->curr.iowait + stats->curr.irq + stats->curr.softirq + stats->curr.steal;
    unsigned long long total_delta = curr_total - prev_total;
    unsigned long long idle_delta = stats->curr.idle - stats->prev.idle;

    if (total_delta == 0) stats->usage_percentage = 0.0;
    else stats->usage_percentage = (double)(total_delta - idle_delta) / total_delta * 100.0;

    stats->prev = stats->curr;
}

void calculate_mem_usage(MemStats *stats) {
    // NEW FORMULA: Total - Available
    // This matches 'top' and 'free' command logic much closer
    long used = stats->total_ram - stats->available_ram;
    
    if (stats->total_ram > 0) stats->percent_used = (double)used / stats->total_ram * 100.0;
    else stats->percent_used = 0.0;
}
