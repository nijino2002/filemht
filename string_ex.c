#include "defs.h"
#include "string_ex.h"

void strex_substr(char *s, char *ss, int pos, int l) {
    return str_substring(s, ss, pos, l);
}

char **strex_split(const char *str, char delim, int *count) {
    if (str == NULL || count == NULL) {
        return NULL;
    }

    *count = 0;
    size_t len = strlen(str);
    if (len == 0) {
        return NULL;
    }

    // calculating the number of delimeter character
    int num_delims = 0;
    for (size_t i = 0; i < len; ++i) {
        if (str[i] == delim) {
            num_delims++;
        }
    }

    *count = num_delims + 1;
    char **result = malloc(*count * sizeof(char *));
    if (result == NULL) {
        *count = 0;
        return NULL;
    }

    int current = 0;
    const char *start = str;
    const char *end = str;

    while (*end) {
        if (*end == delim) {
            size_t substr_len = end - start;
            result[current] = malloc(substr_len + 1);
            if (!result[current]) {
                // 清理已分配内存
                for (int i = 0; i < current; ++i) free(result[i]);
                free(result);
                *count = 0;
                return NULL;
            }
            memcpy(result[current], start, substr_len);
            result[current][substr_len] = '\0';
            current++;
            start = end + 1;
        }
        end++;
    }

    // processing the last string
    size_t substr_len = end - start;
    result[current] = malloc(substr_len + 1);
    if (!result[current]) {
        for (int i = 0; i < current; ++i) free(result[i]);
        free(result);
        *count = 0;
        return NULL;
    }
    memcpy(result[current], start, substr_len);
    result[current][substr_len] = '\0';
    current++;

    return result;
}
