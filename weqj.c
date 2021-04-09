#if defined(__OpenBSD__)

#define _BSD_SOURCE
#else
#define _XOPEN_SOURCE 600
#endif

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct list_str {
  struct list *next;
  char *str;
};

#define WEQJ_NUM_ELEM(x) (sizeof(x) / sizeof(*x))

#define WEQJ_VERSION 0.0.1

#define WEQJ_MAX_NB_SUBS 32
#define WEQJ_MAX_NB_WORDS 2048
#define WEQJ_MAX_NB_DOMAINS 1024

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

static void *xmalloc(size_t z)
{
    void *p = malloc(z);
    if (!p) {
      fprintf(stderr, "out of memory\n");
      exit(EXIT_FAILURE);
    }
    return p;
}

static struct list_str *extract_words2(char *path)
{
  char l[256];
  long n = 0;
  FILE *f = fopen(path, "r");
  struct list_str *head = xmalloc(sizeof(struct list_str));
  struct list_str *tail = head;
  if (!f)
  {
    fprintf(stderr, "failed to open file %s: error %d, %s\n", path, errno,
            strerror(errno));
    return 0;
  }
  while (fgets(l, sizeof(l), f)) {
    l[strcspn(l, "\n")] = 0;
    if (!tail) {
      tail = xmalloc(sizeof(struct list_str));
    }
    tail->str = strdup(l);
    printf("tail->str = %s\n", tail->str);
    tail = tail->next;
    n++;
  }
  tail->next = 0;
  fclose(f);
  return head;
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

static int add_words_from_subdomains(char *domains[], int num_domains,
                                     char *words[], int max_words) {
  int ii, jj, kk;
  char *tmp[256];
  int wordsno, tokenno;
  bool found;
  wordsno = 0;
  for (ii = 0; ii < num_domains; ii++) {
    tokenno = extract_token(domains[ii], ".", tmp, 256);
    found = false;
    for (jj = 0; jj < tokenno; jj++) {
      found = false;
      for (kk = 0; kk < wordsno; kk++) {
        if (strcmp(tmp[jj], words[kk]) == 0) {
          found = true;
        }
      }
      if (found == false) {
        words[wordsno++] = strdup(tmp[jj]);
      }
    }
  }
  return wordsno;
}


int main(int argc, char **argv) {
  char *subs[WEQJ_MAX_NB_SUBS];
  char *words[WEQJ_MAX_NB_SUBS];
  char *domains[WEQJ_MAX_NB_DOMAINS];
  char *file;
  int option, nwords, nsubs, ii, domainsno, n;
  const int max_domains = WEQJ_NUM_ELEM(domains);
  bool read_from_stdin = false;
  nwords = 0;
  while ((option = getopt(argc, argv, "w:hv")) != -1) {
    switch (option) {
    case 'w': {
      nwords = extract_words(optarg, words);
      struct list_str *head = extract_words2(optarg);
      printf("prout\n");
      struct list_str *p = head; 
      while (p->next) {
        printf("words = %s\n", p->str);
        p = p->next;
      }
      #if 0
      for (ii = 0; ii < nwords; ++ii) {
        printf("> %s\n", words[ii]);
      }
      #endif
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
  /* take domain parameter in stdin */
  if (optind < argc) {
    domainsno = 0;
    for (ii = optind; ii < argc; ii++) {
      if (argv[ii][0] == '-' && argv[ii][1] == '\0') {
        if (!read_from_stdin) {
          n = read_line_from_stdin(domains, max_domains - domainsno);
          domainsno += n;
          read_from_stdin = true;
        printf("num of domains = %d\n", domainsno);
        }
      } else {
        domains[domainsno] = strdup(argv[ii]);
        domainsno++;
        assert(domainsno <= max_domains);
      }
    }
  } else if (!isatty(STDIN_FILENO)) {
    domainsno = read_line_from_stdin(domains, max_domains);
  } else {
    fprintf(stderr, "domains are missing\n");
    print_usage(stderr);
    exit(EXIT_FAILURE);
  }

  nwords += add_words_from_subdomains(domains, domainsno, &words[nwords],
                                      WEQJ_MAX_NB_SUBS - nwords);

  for (ii = 0; ii < nwords; ++ii) {
    printf("%d, %s\n", ii, words[ii]);
  }

  free_words(nwords, words);
  return 0;
}
