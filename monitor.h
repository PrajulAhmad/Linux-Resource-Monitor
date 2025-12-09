#ifndef MONITOR_H
#define MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

#define CPU_ALERT_THRESHOLD 50.0
#define MEM_ALERT_THRESHOLD 80.0
#define MAX_TOTAL_PROCESSES 1024  // Store up to 1024 processes in memory
#define DISPLAY_PROCESSES 20      // But only show top 20 on screen

typedef struct {
    long total_ram;
    long free_ram;
    long available_ram; // NEW: More accurate field
    long buffers;
    long cached;
    double percent_used; 
} MemStats;

typedef struct {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
} CpuRawData;

typedef struct {
    CpuRawData prev;
    CpuRawData curr;
    double usage_percentage; 
} CpuStats;

typedef struct {
    int pid;
    char name[256];
    char state;       
    long rss;         
} ProcessInfo;

void read_mem_stats(MemStats *stats);
void read_cpu_stats(CpuStats *stats);
void read_processes(ProcessInfo *procs, int max_storage, int *count);
void calculate_cpu_usage(CpuStats *stats);
void calculate_mem_usage(MemStats *stats);
void sort_processes_by_mem(ProcessInfo *procs, int count);
void check_and_log_alerts(double cpu_usage, double mem_usage);

#endif
