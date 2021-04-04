#if defined(__OpenBSD__)
#define _BSD_SOURCE
#else
#define _XOPEN_SOURCE 600
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WEQJ_VERSION 0.0.1

#define WEQJ_MAX_NB_SUBS 32
#define WEQJ_MAX_NB_WORDS 64000

#define XSTR(s) STR(s)
#define STR(s) #s

#define WEQJ_MAX_BYTES 1024 * 100
static char io_buf[WEQJ_MAX_BYTES];

static void print_version(void) {
    puts("weqj " XSTR(WEQJ_VERSION));
}

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

static int read_stdin(char *domains[], int count) {
    int tokenno, n, ii;
    n = read(STDIN_FILENO, (char *)io_buf, sizeof(io_buf));
    if (n <= 0) 
        return n;
    tokenno = extract_token(io_buf, "\n", domains, count);
    for (ii = 0; ii < tokenno; ii++) {
        printf("domains %d = %s\n", ii, domains[ii]);
    }
    return tokenno;
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
  while ((option = getopt(argc, argv, "d:w:hv")) != -1) {
    switch (option) {
    #if 0
    case 'd': {
      int ii;
      nsubs = extract_subs(optarg, subs);
      printf("nsubs = %d\n", nsubs);
      for (ii = 0; ii < nsubs; ++ii) {
        printf("> %s\n", subs[ii]);
      }
    } break;
    #endif
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

  {
  char *domains[256];
  int domainsno;
  /* FIXME: max 256 */
  /*
  for (ii = optind; ii < argc; ii++) {
      if (argv[ii] == '-') {
        domainsno += read_stdin(domains);
      } 
      domains[domainsno] = strdup(argv[ii])
  }
  */
  domainsno += read_stdin(domains, 256);
  }
  free_words(nwords, words);
  return 0;
}
