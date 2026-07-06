#pragma once

#include <stddef.h>
#include <glib.h>

gboolean read_int_file(const char *path, int *out);
gboolean read_str_file(const char *path, char *out, size_t len);
