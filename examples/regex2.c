#include <stc/cregex.h>
#include <stc/csview.h>
#include <stc/cstr.h>

int main()
{
    const char* inputs[] = {"date: 2024-02-29 leapyear day", "https://en.cppreference.com/w/cpp/regex/regex_search", "!123abcabc!"};
    const char* patterns[] = {"(\\d\\d\\d\\d)-(1[0-2]|0[1-9])-(3[01]|[12][0-9]|0[1-9])",
                              "(https?://|ftp://|www\\.)([0-9A-Za-z@:%_+~#=-]+\\.)+([a-z][a-z][a-z]?)(/[/0-9A-Za-z\\.@:%_+~#=\\?&-]*)?",
                              "!((abc|123)+)!",
    };
    c_forrange (i, c_arraylen(inputs))
    {
        c_auto (cregex, re)
        {
            re = cregex_new(patterns[i], 0);
            cregmatch m[20];
            printf("input: %s\n", inputs[i]);
            if (cregex_find(&re, inputs[i], 20, m, 0) > 0)
            {
                c_forrange (j, cregex_captures(re))
                {
                    csview cap = {m[j].str, m[j].len}; 
                    printf("  submatch %zu: " c_PRIsv "\n", j, c_ARGsv(cap));
                }
                puts("");
            }
        }
    }
}

#include "../src/cregex.c"