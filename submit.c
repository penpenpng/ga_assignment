#pragma region: environment

// fixed parameters
#define MAX_GENE_SIZE 4096
#define MAX_LINE_SIZE 64
#define CALC_TIME 10 * 60

// hyper parameters
#define POPULATION 300
#define BREAK_POINT 10000
#define DEV_MAX_ITERATIONS 10
#define MUTATION_PROB 0.01

// environment variable
#ifdef _WIN64
  #define DEV_ENV
#else
  #define PROD_ENV
#endif

#pragma endregion

#pragma region: include

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#ifdef PROD_ENV
  #include <unistd.h>
#endif

#pragma endregion

#pragma region: declaration global objects

// struct
typedef struct {
  char data[MAX_GENE_SIZE];
  double error;
} Gene;

// global variables
double weight[MAX_GENE_SIZE];
int gene_size;
Gene global_best;

// IO funtions
void load_input_file(char const *path);
void output_the_best();

// algorithm
void start_ga_iteration();
void two_point_crossover(const Gene* parent1, const Gene* parent2, Gene* child1, Gene* child2);
void random_point_crossover(const Gene *parent1, const Gene *parent2, Gene *child1, Gene *child2);
void mutate(Gene* gene);

// utils
void initialize_genes(Gene genes[]);
Gene* random_choice_from(Gene genes[]);
void eval_error(Gene* gene);
void copy_gene(const Gene* src, Gene* dst);
int cmp_gene(const void* gene1, const void* gene2);
int stochastic_bool(double prob);

#pragma endregion

#pragma region: main and IO functions

int main(int argc, char const *argv[]) {
  #ifdef PROD_ENV
    signal(SIGALRM, output_the_best);
    alarm(CALC_TIME);
  #endif

  srand(time(NULL));
  load_input_file(argv[1]);

  global_best.error = DBL_MAX;

  #ifdef PROD_ENV
    for (int i = 0;; i++)
  #else
    for (int i = 0; i < DEV_MAX_ITERATIONS; i++)
  #endif
  {
    printf("# iteration: %d\n", i + 1);
    start_ga_iteration();
  }

  #ifdef DEV_ENV
    output_the_best();
  #endif
  
  return 0;
}

// setup the global variables: `weight` and `gene_size`.
// ONLY this function should modify the above global variables.
void load_input_file(char const *path) {
  FILE *fp = fopen(path, "r");

  if (fp == NULL) {
    perror("Could not open input file.");
    exit(1);
  }

  gene_size = 0;
  char cbuf[MAX_LINE_SIZE];
  while (fgets(cbuf, MAX_LINE_SIZE, fp))
    weight[gene_size++] = atof(cbuf);

  fclose(fp);
}

// print the global variable: global_best.
void output_the_best() {
  for (int i = 0; i < gene_size; i++)
    printf("%d", global_best.data[i]);
  putchar('\n');
  exit(0);
}

#pragma endregion

#pragma region: algorithm

void start_ga_iteration() {
  Gene genes[POPULATION];
  initialize_genes(genes);

  Gene local_best;
  local_best.error = DBL_MAX;

  int gen = 0;

  for (int stagnation = 0; stagnation < BREAK_POINT; gen++, stagnation++) {
    // mutation
    for (int i = 0; i < POPULATION; i++) mutate(&genes[i]);

    // crossover
    Gene *parent1 = random_choice_from(genes);
    Gene* parent2 = random_choice_from(genes);
    Gene child1;
    Gene child2;

    two_point_crossover(parent1, parent2, &child1, &child2);
    // unused: random_point_crossover(parent1, parent2, &child1, &child2);

    // generation change
    Gene* rank[4] = {parent1, parent2, &child1, &child2};
    qsort(rank, 4, sizeof(Gene*), cmp_gene);
    copy_gene(rank[0], rank[0] != parent1 ? parent1 : parent2);
    copy_gene(rank[1], rank[1] != parent1 ? parent1 : parent2);

    // update score and dump log
    if (rank[0]->error < local_best.error) {
      printf("# stag.: %6d, \tlocal update: %.12lf -> %.12lf",
        stagnation,
        local_best.error == DBL_MAX ? INFINITY : local_best.error,
        rank[0]->error);
      copy_gene(rank[0], &local_best);
      stagnation = 0;

      if (local_best.error < global_best.error) {
        printf(", \tglobal update: %.12lf -> %.12lf",
          global_best.error == DBL_MAX ? INFINITY : global_best.error,
          local_best.error);
        copy_gene(&local_best, &global_best);
      }

      printf("\n");
    }
  }

  printf("# gen.:%7d\n\n", gen);
}


void two_point_crossover(const Gene* parent1, const Gene* parent2, Gene* child1, Gene* child2) {
  int a = rand() % gene_size;
  int b = rand() % gene_size;
  int s = a < b ? a : b;
  int e = a < b ? b : a;

  for (int i = 0; i < gene_size; i++) {
    if (s <= i && i <= e) {
      child1->data[i] = parent1->data[i];
      child2->data[i] = parent2->data[i];
    } else {
      child1->data[i] = parent2->data[i];
      child2->data[i] = parent1->data[i];
    }
  }

  eval_error(child1);
  eval_error(child2);
}


void random_point_crossover(const Gene* parent1, const Gene* parent2, Gene* child1, Gene* child2) {
  for (int i = 0; i < gene_size; i++) {
    if (stochastic_bool(0.1)) {
      child1->data[i] = parent1->data[i];
      child2->data[i] = parent2->data[i];
    } else {
      child1->data[i] = parent2->data[i];
      child2->data[i] = parent1->data[i];
    }
  }

  eval_error(child1);
  eval_error(child2);
}


void mutate(Gene* gene) {
  if (!stochastic_bool(MUTATION_PROB)) return;

  gene->data[rand() % gene_size] ^= 1;
  eval_error(gene);
}

#pragma endregion

#pragma region: utils

// setup the first generation genes
void initialize_genes(Gene genes[]) {
  for (int i = 0; i < POPULATION; i++) {
    for (int j = 0; j < gene_size; j++)
      genes[i].data[j] = rand() % 2;

    eval_error(&genes[i]);
  }
}


Gene* random_choice_from(Gene genes[]) {
  return &genes[rand() % POPULATION];
}


void eval_error(Gene* gene) {
  double a = 0;
  double b = 0;

  for (int i = 0; i < gene_size; i++) 
    *(gene->data[i] ? &a : &b) += weight[i];

  gene->error = fabs(a - b);
}


void copy_gene(const Gene* src, Gene* dst) {
  memcpy(dst->data, src->data, sizeof(char) * MAX_GENE_SIZE);
  dst->error = src->error;
}


int cmp_gene(const void* gene1, const void* gene2) {
  return (*(Gene**)gene1)->error > (*(Gene**)gene2)->error ? 1 : -1;
}


int stochastic_bool(double prob) {
  return ((double)rand()) / RAND_MAX >= prob;
}

#pragma endregion
