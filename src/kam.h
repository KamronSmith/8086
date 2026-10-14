#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#define KAMDEF static inline

#define KAM_DECLTYPE(T)

#define da_reserve(da, expected_capacity)                                                                   \
    do {                                                                                                    \
        if ((expected_capacity) > (da)->capacity) {                                                         \
            if ((da)->capacity == 0) {                                                                      \
                (da)->capacity = 256;                                                                       \
            }                                                                                               \
            while ((expected_capacity) > (da)->capacity) {                                                  \
                (da)->capacity *= 2;                                                                        \
            }                                                                                               \
            (da)->items = KAM_DECLTYPE((da)->items)realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            assert((da)->items != NULL);                                                                    \
         }                                                                                                  \
      } while (0)

#define da_append(da, item)                    \
    do {                                       \
        da_reserve((da), (da)->count + 1);     \
        (da)->items[(da)->count++] = (item);   \
} while (0)

#define da_append_many(da, new_items, new_items_count)                                          \
    do {                                                                                        \
        da_reserve((da), (da)->count + (new_items_count));                                      \
        memcpy((da)->items + (da)->count, (new_items), (new_items_count)*sizeof(*(da)->items)); \
        (da)->count += (new_items_count);                                                       \
    } while (0)

#define da_pop(da) (da)->items[(assert((da)->count > 0), --(da)->count)]
#define da_first(da) (da)->items[(assert((da)->count > 0), 0)]
#define da_last(da) (da)->items[(assert((da)->count > 0), (da)->count-1)]
#define da_foreach(Type, it, da) for (Type *it = (da)->items; it < (da)->items + (da)->count; ++it)
#define da_free(da) free((da).items)

#define array_len(array) (sizeof(array)/sizeof(array[0]))
#define array_get(array, index) \
(assert((size_t)index < array_len(array)), array[(size_t)index])

#define TODO(message) do { fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define UNUSED(value) (void)(value)

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define align_up_pow2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (-((u64)(p) - 1)))
#define arena_base_pos (sizeof(arena))
#define arena_align (sizeof(void*))

typedef struct {
    u64 capacity;
    u64 pos;
} mem_arena;

KAMDEF mem_arena *arena_create(u64 capacity) {
    mem_arena *arena = (mem_arena*)malloc(capacity);

    arena->capacity = capacity;
    arena->pos = arena_base_pos;

    return arena;
}

KAMDEF void arena_destroy(mem_arena *arena) {
    free(arena);
}

KAMDEF void *arena_push(mem_arena *arena, u64 size, bool non_zero) {
    u64 pos_aligned = align_up_pow2(arena->pos, arena_align);
    u64 new_pos = pos_aligned + size;

    if (new_pos > arena->capacity) return NULL;

    arena->pos = new_pos;

    u8 *out = (u8*)arena + pos_aligned;

    if (!non_zero) memset(out, 0, size);

    return out;
}

KAMDEF void arena_pop(mem_arena *arena, u64 size) {
    size = min(size, arena->pos - arena_base_pos);
    arena->pos -= size;
}

KAMDEF void arena_pop_to(mem_arena *arena, u64 pos) {
    u64 size = pos < arena->pos ? arena->pos - pos : 0;

    arena_pop(arena, size);
}

KAMDEF void arena_clear(mem_arena *arena) { arena_pop_to(arena, arena_base_pos); }

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String_Builder;

#define sb_append_buf(sb, buf, size) da_append_many(sb, buf, size)
#define sb_append_sv(sb, sv) sb_append_buf((sb), (sv).data, (sv).count)
#define sb_append_null(sb) da_append_many(sb, "", 1)
#define sb_append_cstr(sb, cstr)  \
    do {                              \
        const char *s = (cstr);       \
        size_t n = strlen(s);         \
        da_append_many(sb, s, n); \
    } while (0)
#define sb_append da_append

typedef struct {
    size_t count;
    const char *data;
} String;

KAMDEF String string_from_cstr(const char *cstr) {
    return (String) {
        .count = strlen(cstr),
        .data = cstr,
    };
}

KAMDEF String string_from_parts(const char *data, size_t count) {
    String sv;
    sv.count = count;
    sv.data = data;
    return sv;
}

KAMDEF bool string_eq(String a, String b) {
    if (a.count != b.count) {
        return false;
    } else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

KAMDEF bool string_starts_with(String str, String expected_prefix) {
    if (expected_prefix.count <= str.count) {
        String actual_prefix = string_from_parts(str.data, expected_prefix.count);
        return string_eq(expected_prefix, actual_prefix);
    }

    return false;
}

KAMDEF bool string_ends_with(String str, String suffix) {
    if (str.count >= suffix.count) {
        String sv_tail = {
            .count = suffix.count,
            .data = str.data + str.count - suffix.count,
        };
        return string_eq(sv_tail, suffix);
    }
    return false;
}

KAMDEF bool str_ends_with_cstr(String str, const char *cstr) {
    return string_ends_with(str, string_from_cstr(cstr));
}

KAMDEF String string_chop_left(String *str, size_t n) {
    if (n > str->count) {
        n = str->count;
    }

    String result = string_from_parts(str->data, n);

    str->data  += n;
    str->count -= n;

    return result;
}

KAMDEF String string_chop_right(String *str, size_t n) {
    if (n > str->count) {
        n = str->count;
    }

    String result = string_from_parts(str->data + str->count - n, n);

    str->count -= n;

    return result;
}

KAMDEF String string_chop_by_delim(String *str, char delim) {
    size_t i = 0;
    while (i < str->count && str->data[i] != delim) {
        i += 1;
    }

    String result = string_from_parts(str->data, i);

    if (i < str->count) {
        str->count -= i + 1;
        str->data  += i + 1;
    } else {
        str->count -= i;
        str->data  += i;
    }

    return result;
}

KAMDEF bool string_chop_prefix(String *str, String prefix) {
    if (string_starts_with(*str, prefix)) {
        string_chop_left(str, prefix.count);
        return true;
    }
    return false;
}

KAMDEF bool string_chop_suffix(String *str, String suffix)
{
    if (string_ends_with(*str, suffix)) {
        string_chop_right(str, suffix.count);
        return true;
    }
    return false;
}

KAMDEF String string_trim_left(String str)
{
    size_t i = 0;
    while (i < str.count && isspace(str.data[i])) {
        i += 1;
    }

    if (i == 0) return str;
    return string_from_parts(str.data + i, str.count - i);
}

KAMDEF String string_trim_right(String str) {
    size_t i = 0;
    while (i < str.count && isspace(str.data[str.count - 1 - i])) {
        i += 1;
    }

    return string_from_parts(str.data, str.count - i);
}

KAMDEF String string_trim(String str)
{
    return string_trim_right(string_trim_left(str));
}

KAMDEF int string_find(String haystack, String needle) {
    for (size_t i = 0; needle.count < haystack.count; i++) {
        if (string_starts_with(haystack, needle)) {
            return (int)i;
        }
    }
    return -1;
}
