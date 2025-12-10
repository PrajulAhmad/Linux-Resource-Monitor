// Minimal logger stub (not required for core monitor)
#include <stdio.h>

void log_alert(const char *msg, double val) {
    FILE *f = fopen("system_monitor.log", "a");
    if (!f) return;
    fprintf(f, "%s: %.2f\n", msg, val);
    fclose(f);
}
