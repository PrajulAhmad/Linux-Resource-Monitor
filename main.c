#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "monitor.h"

#define REFRESH_MS 2000
#define POLL_MS 100

static struct termios orig_term;
static volatile sig_atomic_t sigint_flag = 0;

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
    printf("\033[?25h\033[?1049l");
    fflush(stdout);
}

void sigint_handler(int signo) {
    (void)signo;
    sigint_flag = 1;
    restore_terminal();
    _exit(0);
}

void enable_raw() {
    tcgetattr(STDIN_FILENO, &orig_term);
    atexit(restore_terminal);

    struct termios raw = orig_term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // non-blocking input
    int fl = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK);

    // clear input buffer
    tcflush(STDIN_FILENO, TCIFLUSH);

    // full-screen alt buffer + hide cursor
    printf("\033[?1049h\033[H\033[?25l");
    fflush(stdout);

    // install SIGINT handler
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

int sort_cpu(const void *a, const void *b) {
    const ProcessInfo *p1 = a, *p2 = b;
    if (p2->cpu_usage > p1->cpu_usage) return 1;
    if (p2->cpu_usage < p1->cpu_usage) return -1;
    return 0;
}

void draw_screen() {
    GlobalStats *g = get_global_stats();
    ProcessInfo *plist = get_process_list();
    int count = get_process_count();

    // Clear full screen each redraw (prevents leftover junk)
    printf("\033[H\033[2J");

    printf("LINUX RESOURCE MONITOR (Press 'q' to quit)\n");
    printf("==============================================================\n");

    double cpu = g->cpu_percent;
    if (cpu < 0) cpu = 0;
    if (cpu > 100) cpu = 100;

    int bars = cpu / 5.0;
    printf("CPU: [");
    for (int i = 0; i < 20; i++) printf(i < bars ? "|" : " ");
    printf("] %5.1f%%\n", cpu);

    double mem = g->mem_percent;
    if (mem < 0) mem = 0;
    if (mem > 100) mem = 100;

    bars = mem / 5.0;
    printf("MEM: [");
    for (int i = 0; i < 20; i++) printf(i < bars ? "|" : " ");
    printf("] %5.1f%% (%lu/%lu MB)\n",
           mem, g->used_mem_kb/1024, g->total_mem_kb/1024);

    printf("--------------------------------------------------------------\n");
    printf("%-8s %-20s %-10s %-7s\n", "PID", "NAME", "MEM(MB)", "CPU%");

    qsort(plist, count, sizeof(ProcessInfo), sort_cpu);

    int limit = (count < 20 ? count : 20);

    for (int i = 0; i < limit; i++) {
        // truncate long names to 20 chars
        char name20[21];
        strncpy(name20, plist[i].name, 20);
        name20[20] = '\0';

        double cpu_p = plist[i].cpu_usage;
        if (cpu_p < 0) cpu_p = 0.0;
        if (cpu_p > 100) cpu_p = 100;

        printf("%-8d %-20s %-10.1f %-7.2f\n",
               plist[i].pid,
               name20,
               plist[i].memory_kb / 1024.0,
               cpu_p);
    }

    fflush(stdout);
}

int main() {
    enable_raw();
    update_stats();

    while (1) {
        draw_screen();
        update_stats();

        int loops = REFRESH_MS / POLL_MS;

        for (int i = 0; i < loops; i++) {
            char c;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                if (c == 'q' || c == 'Q') {
                    restore_terminal();
                    return 0;
                }
            }
            if (sigint_flag) {
                restore_terminal();
                return 0;
            }
            usleep(POLL_MS * 1000);
        }
    }
}
