#include <stdio.h>
#include <signal.h> 
#include "monitor.h"

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
    (void)sig; 
    keep_running = 0; 
}

// Helper: Enables "Alternate Screen" (Like vim/htop)
// This PREVENTS scrolling history
void set_alt_screen(int enable) {
    if (enable) {
        printf("\033[?1049h"); // Save terminal and switch to buffer
        printf("\033[H");      // Move Home
    } else {
        printf("\033[?1049l"); // Restore original terminal
    }
    fflush(stdout);
}

void print_stats(CpuStats *cpu, MemStats *mem, ProcessInfo *procs, int proc_count) {
    // Move to top-left (Do not use clear)
    printf("\033[H"); 
    
    printf("=== Linux Resource Monitor ===\n");
    printf("CPU Usage:    %.2f %%   \n", cpu->usage_percentage);
    printf("Memory Usage: %.2f %% (Real Used)\n", mem->percent_used);
    
    // Alert Section
    if (cpu->usage_percentage > CPU_ALERT_THRESHOLD) 
        printf(" [!] CPU ALERT LOGGED   \n");
    else 
        printf("                        \n"); 

    printf("--------------------------------------------\n");
    printf("%-8s %-6s %-12s %s\n", "PID", "State", "Mem (MB)", "Name");
    printf("--------------------------------------------\n");
    
    // Display limit
    int limit = (proc_count < DISPLAY_PROCESSES) ? proc_count : DISPLAY_PROCESSES;
    
    for (int i = 0; i < limit; i++) {
        double mem_mb = procs[i].rss / 1024.0;
        // \033[K clears the rest of the line (cleans up old long names)
        printf("%-8d %-6c %-12.2f %s\033[K\n", procs[i].pid, procs[i].state, mem_mb, procs[i].name);
    }
    
    // Clear everything below our list (in case list shrank)
    printf("\033[J");

    printf("--------------------------------------------\n");
    printf("Total Processes: %d\n", proc_count);
    printf("Press CTRL+C to quit safely.\n");
    fflush(stdout);
}

int main() {
    signal(SIGINT, handle_sigint);

    // 1. Enter Alternate Screen Mode (Stops scrolling!)
    set_alt_screen(1);

    MemStats m_stats = {0};
    CpuStats c_stats = {0};
    static ProcessInfo procs[MAX_TOTAL_PROCESSES];
    int proc_count = 0;

    read_cpu_stats(&c_stats);
    c_stats.prev = c_stats.curr; 

    while (keep_running) {
        read_mem_stats(&m_stats);
        read_cpu_stats(&c_stats);
        read_processes(procs, MAX_TOTAL_PROCESSES, &proc_count);
        sort_processes_by_mem(procs, proc_count);
        calculate_cpu_usage(&c_stats);
        calculate_mem_usage(&m_stats);
        check_and_log_alerts(c_stats.usage_percentage, m_stats.percent_used);

        print_stats(&c_stats, &m_stats, procs, proc_count);
        
        sleep(1);
    }

    // 2. Restore Terminal on Exit
    set_alt_screen(0);
    printf("Exiting... Logs saved.\n");
    return 0;
}
