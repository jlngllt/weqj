#if defined(__OpenBSD__)
#  define _BSD_SOURCE
#else
#  define _XOPEN_SOURCE 600
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NB_SUBS 32

#define max(a,b)  ((a) > (b) ? (a) : (b))
#define min(a,b)  ((a) < (b) ? (a) : (b))

static int
extract_subs(char *domain, char *subs[]) {
    int ntokens = 0;
    char *save;
    char *ptr = domain;
    for (; ntokens < MAX_NB_SUBS; ++ntokens) {
        char *tok = strtok_r(ptr, ".", &save);
        if (!tok)
            break;
        subs[ntokens] = tok;
        ptr = save;
    }
    return ntokens;
}

static void
usage(FILE *f) {
    fprintf(f, "Usage: weqj [-vh] [-w WORDS] [-d DOMAINS]\n");
}

int main(int argc, char **argv) {
    char domain[] = "www.sub.blog.admin.domain.tld";
    char *subs[MAX_NB_SUBS];

    int option;
    while ((option = getopt(argc, argv, "d:w:h")) != -1) {
        switch(option) {
            case 'd': {
                          int nsubs = extract_subs(domain, subs);
                          int ii = 0;
                          printf("nsubs = %d\n", nsubs);
                          for (ii = 0; ii < nsubs; ++ii) {
                            printf("> %s\n", subs[ii]);
                          }
                      } break;
            case 'h': {
                          usage(stdout);
                          exit(EXIT_SUCCESS);
                      } break;
        }
    }
    return 0;
}
