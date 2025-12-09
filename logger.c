#include "monitor.h"

#define LOG_COOLDOWN 5
time_t last_log_time = 0;

void log_alert(const char *msg, double value) {
    FILE *fp = fopen("system_monitor.log", "a"); 
    if (!fp) return;

    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; 

    fprintf(fp, "[%s] ALERT: %s (Value: %.2f%%)\n", time_str, msg, value);
    fclose(fp);
}

void check_and_log_alerts(double cpu_usage, double mem_usage) {
    time_t now = time(NULL);
    if (difftime(now, last_log_time) < LOG_COOLDOWN) return;

    int triggered = 0;
    if (cpu_usage > CPU_ALERT_THRESHOLD) {
        log_alert("High CPU Usage", cpu_usage);
        triggered = 1;
    }
    if (mem_usage > MEM_ALERT_THRESHOLD) {
        log_alert("High Memory Usage", mem_usage);
        triggered = 1;
    }
    if (triggered) last_log_time = now;
}
