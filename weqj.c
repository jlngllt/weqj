#if defined(__OpenBSD__)
#define _BSD_SOURCE
#else
#define _XOPEN_SOURCE 600
#endif

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WEQJ_NUM_ELEM(x) (sizeof(x) / sizeof(*x))

#define WEQJ_VERSION 0.0.1

#define WEQJ_MAX_NB_SUBS 32
#define WEQJ_MAX_NB_WORDS 64000

#define XSTR(s) STR(s)
#define STR(s) #s

#define WEQJ_MAX_BYTES 1024 * 100
static char io_buf[WEQJ_MAX_BYTES];

static void print_version(void) { puts("weqj " XSTR(WEQJ_VERSION)); }

static int extract_token(char *in, char *delim, char *out[], int count) {
  int ntokens = 0;
  char *save;
  char *ptr = in;
  for (; ntokens < count; ++ntokens) {
    char *tok = strtok_r(ptr, delim, &save);
    if (!tok)
      break;
    out[ntokens] = tok;
    ptr = save;
  }
  return ntokens;
}

static long extract_words(char *path, char *words[]) {
  char line[256];
  long lineno = 0;
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "failed to open file %s: error %d, %s\n", path, errno,
            strerror(errno));
    return 0;
  }
  while (fgets(line, sizeof(line), f)) {
    line[strcspn(line, "\n")] = 0;
    words[lineno] = strdup(line);
    lineno++;
    if (lineno >= WEQJ_MAX_NB_WORDS)
      break;
  }
  fclose(f);
  return lineno;
}

static void free_words(int n, char *words[]) {
  int ii;
  for (ii = 0; ii < n; ++ii) {
    free(words[ii]);
  }
}

static int read_line_from_stdin(char *domains[], int count) {
  int tokenno, n, ii;
  n = read(STDIN_FILENO, (char *)io_buf, sizeof(io_buf));
  if (n <= 0)
    return n;
  return extract_token(io_buf, "\n", domains, count);
}

static void print_usage(FILE *f) {
  fprintf(f, "Usage: weqj [-vh] [-w words file] [domains...]\n");
}

int main(int argc, char **argv) {
  char *subs[WEQJ_MAX_NB_SUBS];
  char *words[WEQJ_MAX_NB_SUBS];
  char *file;
  int option, nwords, nsubs, ii;
  nwords = 0;
  while ((option = getopt(argc, argv, "w:hv")) != -1) {
    switch (option) {
    case 'w': {
      nwords = extract_words(optarg, words);
      printf("nwords = %d\n", nwords);
      for (ii = 0; ii < nwords; ++ii) {
        printf("> %s\n", words[ii]);
      }
    } break;
    case 'h': {
      print_usage(stdout);
      exit(EXIT_SUCCESS);
    } break;
    case 'v': {
      print_version();
      exit(EXIT_SUCCESS);
    } break;
    default: {
      print_usage(stderr);
      exit(EXIT_FAILURE);
    }
    }
  }

  { /* test */
    char *domains[256];
    const int max_domains = WEQJ_NUM_ELEM(domains);
    int domainsno, n;
    bool read_from_stdin = false;
    if (optind < argc) {
      domainsno = 0;
      for (ii = optind; domainsno < max_domains, ii < argc; ii++) {
        if (argv[ii][0] == '-' && argv[ii][1] == '\0') {
          if (!read_from_stdin) {
            n = read_line_from_stdin(domains, max_domains - domainsno);
            domainsno += n;
            read_from_stdin = true;
          }
        } else {
          domains[domainsno] = strdup(argv[ii]);
          domainsno++;
        }
      }
    } else if (!isatty(STDIN_FILENO)) {
      domainsno = read_line_from_stdin(domains, max_domains);
    } else {
      fprintf(stderr, "domains are missing\n");
      print_usage(stderr);
      exit(EXIT_FAILURE);
    }
    for (ii = 0; ii < domainsno; ii++) {
        printf("%s\n", domains[ii]);
    }
  }
  free_words(nwords, words);
  return 0;
}
