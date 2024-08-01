#define i_implement
#include "stc/cstr.h"
#define i_key_cstr
#define i_val_cstr
#include "stc/hmap.h"

int main(void)
{
    hmap_cstr m = {0};
    hmap_cstr_emplace(&m, "Test short", "This is a short string");
    hmap_cstr_emplace(&m, "Test long ", "This is a longer string");

    c_foreach_kv (k, v, hmap_cstr, m)
        printf("%s: '%s' Len=%d, Is long: %s\n",
                cstr_str(k), cstr_str(v), (int)cstr_size(v),
                cstr_is_long(v) ? "true" : "false");

    hmap_cstr_drop(&m);
}
