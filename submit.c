#pragma region: environment

#ifndef DEV_ENV
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

#define MAX_GENE_SIZE 4096
#define POPULATION 500
#define MAX_LINE_SIZE 64
#define CALC_TIME 10 * 60

double weight[MAX_GENE_SIZE];
int gene_size;
char best_gene[MAX_GENE_SIZE];


// IO funtions
void load_input_file(char const *path);
void output_the_best();

// algorithm
void calc();
void initialize(char genes[POPULATION][MAX_GENE_SIZE]);
void two_point_crossover(const char* parent1, const char* parent2, char* child1, char* child2);
void mutate(char* gene);
char* random_gene(char genes[POPULATION][MAX_GENE_SIZE]);
double eval_error(const char* gene);

// utils
void copy_gene(const char* src, char* dst);

#pragma endregion

#pragma region: main and IO functions

int main(int argc, char const *argv[])
{
  load_input_file(argv[1]);

  #ifdef PROD_ENV
    signal(SIGALRM, output_the_best);
    alarm(CALC_TIME);
  #endif

  srand(time(NULL));
  calc();
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
    printf("%d", best_gene[i]);
  putchar('\n');
  exit(0);
}

#pragma endregion


void calc() {
  char genes[POPULATION][MAX_GENE_SIZE];
  char child1[MAX_GENE_SIZE];
  char child2[MAX_GENE_SIZE];
  char* parent1;
  char* parent2;
  double child1_err, child2_err, parent1_err, parent2_err, min_err, err;
  initialize(genes);

  min_err = DBL_MAX;
  for (int i = 0; i < POPULATION; i++) {
    err = eval_error(genes[i]);
    if (err < min_err) {
      min_err = err;
      copy_gene(genes[i], best_gene);
    }
  }

  #ifdef DEV_ENV
    #define STOP_CONDITION i < 1000
  #else
    #define STOP_CONDITION
  #endif
  for (int i = 0; STOP_CONDITION;i++) {
    parent1 = random_gene(genes);
    parent2 = random_gene(genes);
    parent1_err = eval_error(parent1);
    parent2_err = eval_error(parent2);

    two_point_crossover(parent1, parent2, child1, child2);
    mutate(child1);
    mutate(child2);

    child1_err = eval_error(child1);
    child2_err = eval_error(child2);

    if (child1_err < parent1_err)
      copy_gene(child1, parent1);
    if (child2_err < parent2_err)
      copy_gene(child2, parent2);

    if (child1_err < min_err) {
      min_err = child1_err;
      copy_gene(parent1, best_gene);
    }
    if (child2_err < min_err) {
      min_err = child2_err;
      copy_gene(parent2, best_gene);
    }

    printf("# [%d] min_err: %lf\n", i, min_err);
  }
}


void initialize(char genes[POPULATION][MAX_GENE_SIZE]) {
  for (int i = 0; i < POPULATION; i++)
    for (int j = 0; j < gene_size; j++)
      genes[i][j] = rand() % 2;
}


void two_point_crossover(const char* parent1, const char* parent2, char* child1, char* child2) {
  int a = rand() % gene_size;
  int b = rand() % gene_size;
  int s = a < b ? a : b;
  int e = a < b ? b : a;

  for (int i = 0; i < gene_size; i++) {
    if (s <= i && i <= e) {
      child1[i] = parent1[i];
      child2[i] = parent2[i];
    } else {
      child1[i] = parent2[i];
      child2[i] = parent1[i];
    }
  }
}


void mutate(char* gene) {
  // pass
}


char* random_gene(char genes[POPULATION][MAX_GENE_SIZE]) {
  return genes[rand() % POPULATION];
}


double eval_error(const char* gene) {
  double a = 0;
  double b = 0;

  for (int i = 0; i < gene_size; i++) {
    if (gene[i]) a += weight[i];
    else         b += weight[i];
  }

  return fabs(a - b);
}


void copy_gene(const char* src, char* dst) {
  memcpy(dst, src, sizeof(char) * MAX_GENE_SIZE);
}
