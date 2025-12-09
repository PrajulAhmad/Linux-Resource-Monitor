#include "monitor.h"

int is_numeric(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

void read_mem_stats(MemStats *stats) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "MemTotal:", 9) == 0) sscanf(line, "MemTotal: %ld kB", &stats->total_ram);
        else if (strncmp(line, "MemAvailable:", 13) == 0) sscanf(line, "MemAvailable: %ld kB", &stats->available_ram);
    }
    fclose(fp);
}

void read_cpu_stats(CpuStats *stats) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return;
    char line[256];
    if (fgets(line, sizeof(line), fp)) {
        // FIX: Using "cpu %llu" (one space) matches ANY amount of whitespace.
        // This prevents failures if the kernel uses 1 space vs 2 spaces.
        sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &stats->curr.user, &stats->curr.nice, &stats->curr.system,
               &stats->curr.idle, &stats->curr.iowait, &stats->curr.irq,
               &stats->curr.softirq, &stats->curr.steal);
    }
    fclose(fp);
}

void read_processes(ProcessInfo *procs, int max_storage, int *count) {
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    if (!dir) return;

    *count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (*count >= max_storage) break;
        if (!is_numeric(entry->d_name)) continue;

        char path[512];
        snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
        
        FILE *fp = fopen(path, "r");
        if (fp) {
            int pid;
            char comm[256];
            char state;
            if (fscanf(fp, "%d %s %c", &pid, comm, &state) == 3) {
                int dummy;
                for(int i = 0; i < 20; i++) fscanf(fp, "%d", &dummy);
                long rss_pages;
                long rss_kb = 0;
                if(fscanf(fp, "%ld", &rss_pages) == 1) rss_kb = rss_pages * 4;

                if (rss_kb > 0) {
                    procs[*count].pid = pid;
                    if (comm[0] == '(') {
                        size_t len = strlen(comm);
                        comm[len-1] = '\0';
                        strcpy(procs[*count].name, comm + 1);
                    } else {
                        strcpy(procs[*count].name, comm);
                    }
                    procs[*count].state = state;
                    procs[*count].rss = rss_kb;
                    (*count)++;
                }
            }
            fclose(fp);
        }
    }
    closedir(dir);
}
