#include <criterion/criterion.h>
#include <criterion/hooks.h>
#include <criterion/internal/assert.h>
#include <stddef.h>
#include "parse.h"

Test(parse_instruction, failing) {
    cr_assert_eq(add_three(2), 5);
}
