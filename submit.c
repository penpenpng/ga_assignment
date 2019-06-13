#pragma region: environment

// fixed parameters
#define MAX_GENE_SIZE 4096
#define MAX_LINE_SIZE 64
#define CALC_TIME 10 * 60

// hyper parameters
#define POPULATION 500
#define BREAK_POINT 10000
#define DEV_MAX_ITERATIONS 3
#define DEV_MAX_GENERATIONS 50000

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
Gene best_gene;

// IO funtions
void load_input_file(char const *path);
void output_the_best();

// algorithm
void start_ga_iteration();
void two_point_crossover(const Gene* parent1, const Gene* parent2, Gene* child1, Gene* child2);
void mutate(Gene* gene);

// utils
void initialize_genes(Gene genes[]);
void copy_gene(const Gene* src, Gene* dst);
Gene* random_choice_from(Gene genes[]);
void eval_error(Gene* gene);

#pragma endregion

#pragma region: main and IO functions

int main(int argc, char const *argv[]) {
  #ifdef PROD_ENV
    signal(SIGALRM, output_the_best);
    alarm(CALC_TIME);
  #endif

  srand(time(NULL));

  best_gene.error = DBL_MAX;
  load_input_file(argv[1]);

  #ifdef PROD_ENV
    for (;;)
  #else
    for (int i = 0; i < DEV_MAX_ITERATIONS; i++)
  #endif
  start_ga_iteration();
  
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

// print the global variable: best_gene.
void output_the_best() {
  for (int i = 0; i < gene_size; i++)
    printf("%d", best_gene.data[i]);
  putchar('\n');
  exit(0);
}

#pragma endregion

#pragma region: algorithm

void start_ga_iteration() {
  printf("# start new iteration\n");

  Gene genes[POPULATION];
  initialize_genes(genes);

  int stagnation = 0;

  #ifdef PROD_ENV
    for (int gen = 0;; gen++)
  #else
    for (int gen = 0; gen < DEV_MAX_GENERATIONS; gen++)
  #endif
  {
    Gene* parent1 = random_choice_from(genes);
    Gene* parent2 = random_choice_from(genes);
    Gene child1;
    Gene child2;

    two_point_crossover(parent1, parent2, &child1, &child2);
    mutate(&child1);
    mutate(&child2);

    if (child1.error < parent1->error)
      copy_gene(&child1, parent1);
    if (child2.error < parent2->error)
      copy_gene(&child2, parent2);

    if (child1.error < best_gene.error) {
      copy_gene(parent1, &best_gene);
      stagnation = 0;
    }
    else if (child2.error < best_gene.error) {
      copy_gene(parent2, &best_gene);
      stagnation = 0;
    } else {
      stagnation++;
    }

    if (stagnation > BREAK_POINT)
      return;

    printf("# Gen.:%d(%d) score: %lf\n", gen, stagnation, best_gene.error);
  }
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


void mutate(Gene* gene) {
  // pass
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

#pragma endregion
