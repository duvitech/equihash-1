#define _XOPEN_SOURCE 700
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sodium.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <assert.h>
#include "equihash.h"

double get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}



int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "%s <ierations>\n", argv[0]);
    exit(1);
  }

  gpu_config_t *config = calloc(sizeof(gpu_config_t), 1);
  equihash_init(config);

  sodium_init();
  size_t n_iterations = strtoul(argv[1], NULL, 10);

  crypto_generichash_blake2b_state curr_state;
  crypto_generichash_blake2b_init(&curr_state, (const unsigned char *) "", 0,
                                  (512 / 200) * 200 / 8);

  size_t total_solutions = 0;
  double start_time = get_time();

  for (uint32_t i = 0; i < n_iterations; ++i) {
    crypto_generichash_blake2b_state state = curr_state;
    crypto_generichash_blake2b_update(&state, (const uint8_t *) &i, 4);
    uint32_t dst_indices[20 * 512];
    size_t n_solutions = equihash(dst_indices, &state, config);
    total_solutions += n_solutions;

    for (size_t i = 0; i < n_solutions; ++i) {
      for (size_t k = 0; k < 512; ++k) {
        printf("%u ", dst_indices[i * 512 + k]);
      }
      printf("\n\n");
    }
  }
  double total_time = get_time() - start_time;
  fprintf(stdout, "avg. time: %lf\n", total_time / n_iterations);
  fprintf(stdout, "tot. time: %lf\n", total_time);
  fprintf(stdout, "%lf sol/s\n", total_solutions / total_time);
  fprintf(stdout, "total solutions: %zu\n", total_solutions);

  equihash_cleanup(config);
  free(config);
  return 0;
}
