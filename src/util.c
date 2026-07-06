#include "util.h"
#include <stdio.h>
#include <string.h>

gboolean read_int_file(const char *path, int *out) {
    FILE *f = fopen(path, "r");
    if (!f) return FALSE;
    gboolean ok = (fscanf(f, "%d", out) == 1);
    fclose(f);
    return ok;
}

gboolean read_str_file(const char *path, char *out, size_t len) {
    FILE *f = fopen(path, "r");
    if (!f) return FALSE;
    if (!fgets(out, (int)len, f)) { fclose(f); return FALSE; }
    fclose(f);
    out[strcspn(out, "\n")] = '\0';
    return TRUE;
}
