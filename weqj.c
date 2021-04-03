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

#define MAX_NB_SUBS 32
#define MAX_NB_WORDS 64000

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

static int extract_subs(char *domain, char *subs[]) {
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
    if (lineno >= MAX_NB_WORDS)
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

static void usage(FILE *f) {
  fprintf(f, "Usage: weqj [-vh] [-w WORDS] [-d DOMAINS]\n");
}

int main(int argc, char **argv) {
  char *subs[MAX_NB_SUBS];
  char *words[MAX_NB_SUBS];
  int option;
  int nwords = 0;
  int nsubs = 0;
  while ((option = getopt(argc, argv, "d:w:h")) != -1) {
    switch (option) {
    case 'd': {
      int ii;
      nsubs = extract_subs(optarg, subs);
      printf("nsubs = %d\n", nsubs);
      for (ii = 0; ii < nsubs; ++ii) {
        printf("> %s\n", subs[ii]);
      }
    } break;
    case 'w': {
      int ii;
      nwords = extract_words(optarg, words);
      printf("nwords = %d\n", nwords);
      for (ii = 0; ii < nwords; ++ii) {
        printf("> %s\n", words[ii]);
      }
    } break;
    case 'h': {
      usage(stdout);
      exit(EXIT_SUCCESS);
    } break;
    }
  }
  free_words(nwords, words);
  return 0;
}
