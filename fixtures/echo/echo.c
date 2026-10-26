#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define ECHO_EXPORT __declspec(dllexport)
#else
#define ECHO_EXPORT __attribute__((visibility("default")))
#endif

static int32_t fail_with(const char *message, const char *fix, uint8_t **out, size_t *out_len) {
    static const char template[] = "{\"message\":\"%s\",\"fix\":\"%s\"}";
    size_t capacity = sizeof(template) + strlen(message) + strlen(fix);
    char *body = malloc(capacity);
    if (body == NULL) {
        *out = NULL;
        *out_len = 0;
        return 2;
    }
    int written = snprintf(body, capacity, template, message, fix);
    *out = (uint8_t *)body;
    *out_len = (size_t)written;
    return 1;
}

static int32_t emit(const uint8_t *bytes, size_t length, uint8_t **out, size_t *out_len) {
    uint8_t *copy = malloc(length == 0 ? 1 : length);
    if (copy == NULL) {
        *out = NULL;
        *out_len = 0;
        return 2;
    }
    if (length > 0) {
        memcpy(copy, bytes, length);
    }
    *out = copy;
    *out_len = length;
    return 0;
}

ECHO_EXPORT int32_t echo_echo(const uint8_t *in, size_t in_len, uint8_t **out, size_t *out_len) {
    return emit(in, in_len, out, out_len);
}

ECHO_EXPORT int32_t echo_fail(const uint8_t *in, size_t in_len, uint8_t **out, size_t *out_len) {
    (void)in;
    (void)in_len;
    return fail_with("echo refused on purpose", "call echo instead", out, out_len);
}

ECHO_EXPORT int32_t echo_repeat(const uint8_t *in, size_t in_len, uint8_t **out, size_t *out_len) {
    char *text = malloc(in_len + 1);
    if (text == NULL) {
        *out = NULL;
        *out_len = 0;
        return 2;
    }
    memcpy(text, in, in_len);
    text[in_len] = '\0';

    const char *key = strstr(text, "\"count\"");
    const char *colon = key == NULL ? NULL : strchr(key, ':');
    long count = -1;
    if (colon != NULL) {
        char *end = NULL;
        count = strtol(colon + 1, &end, 10);
        if (end == colon + 1) {
            count = -1;
        }
    }
    free(text);

    if (count < 0) {
        return fail_with("repeat needs a non-negative count", "pass {\\\"count\\\": 3}", out, out_len);
    }

    size_t length = (size_t)count + 2;
    uint8_t *body = malloc(length);
    if (body == NULL) {
        *out = NULL;
        *out_len = 0;
        return 2;
    }
    body[0] = '"';
    memset(body + 1, 'x', (size_t)count);
    body[length - 1] = '"';
    *out = body;
    *out_len = length;
    return 0;
}

ECHO_EXPORT int32_t echo_garbage(const uint8_t *in, size_t in_len, uint8_t **out, size_t *out_len) {
    static const uint8_t invalid_utf8[] = {0xFF, 0xFE};
    (void)in;
    (void)in_len;
    return emit(invalid_utf8, sizeof(invalid_utf8), out, out_len);
}

ECHO_EXPORT void echo_release(uint8_t *buffer) {
    free(buffer);
}
