#include "transducers.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void string_stream(const void *arg, FILE *out) {
  fputs((const char*) arg, out);
}

void increment_stream(const void *arg, FILE *out, FILE *in) {
  int d = *(int*)arg;

  unsigned char c;
  while (fread(&c, sizeof(unsigned char), 1, in) == 1) {
    c += d;
    if (fwrite(&c, sizeof(unsigned char), 1, out) != 1) {
      break;
    }
  }
}

int main() {
  stream* s[2];

  char *input = "Hello, World!";
  char *output = malloc(strlen(input)+1);
  output[strlen(input)] = '\0'; /* Ensure terminating NULL. */
  int inc = 1;

  assert(transducers_link_source(&s[0], string_stream, input) == 0);
  assert(transducers_link_1(&s[1], increment_stream, &inc, s[0]) == 0);
  assert(transducers_link_1(&s[1], increment_stream, &inc, s[0]) == 1); // Fails, since s[0] is already in use.

  /* Note the sizeof()-trick to determine the number of elements in
     the array.  This *only* works for statically allocated arrays,
     *not* ones created by malloc(). */
  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[0])); i++) {
    transducers_free_stream(s[i]);
  }

  free(output);

  return 0;
}
