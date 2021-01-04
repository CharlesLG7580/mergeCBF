/*
 * test.cc
 *
 * Copyright (c) 2014 Vedant Kumar <vsk@berkeley.edu>
 */
#pragma warning(disable:4996)
#define SALT_CONSTANT 0x97c29b3a
extern "C" {
  
  #include "murmur.cpp"
}
#include "qf.c"
#define QBENCH 0
#include <set>
#include <vector>
#include <cassert>
#include <cstdio>
#include <cmath>
#include <time.h>
#include"string"
#include<stdio.h>
#include<stdlib.h>

using namespace std;

/* I need a more powerful machine to increase these parameters... */
const uint32_t Q_MAX = 12;
const uint32_t R_MAX = 6;
const uint32_t ROUNDS_MAX = 1000;

static void fail(struct quotient_filter *qf, const char *s)
{
  fprintf(stderr, "qf(q=%u, r=%u): %s\n", qf->qf_qbits, qf->qf_rbits, s);
  abort();
}

static uint64_t rand64()
{
  return (((uint64_t) rand()) << 32) | ((uint64_t) rand());
}

static void qf_print(struct quotient_filter *qf)
{
  char buf[32];
  uint32_t pad = uint32_t(ceil(float(qf->qf_qbits) / logf(10.f))) + 1;

  for (uint32_t i = 0; i < pad; ++i) {
    printf(" ");
  }
  printf("| is_shifted | is_continuation | is_occupied | remainder"
      " nel=%u\n", qf->qf_entries);

  for (uint64_t idx = 0; idx < qf->qf_max_size; ++idx) {
    _snprintf(buf, sizeof(buf), "%llu", idx);
    printf("%s", buf);

    int fillspace = pad - strlen(buf);
    for (int i = 0; i < fillspace; ++i) {
      printf(" ");
    }
    printf("| ");

    uint64_t elt = get_elem(qf, idx);
    printf("%d          | ", !!is_shifted(elt));
    printf("%d               | ", !!is_continuation(elt));
    printf("%d           | ", !!is_occupied(elt));
    printf("%llu\n", get_remainder(elt));
  }
}

/* Check QF structural invariants. */
static void qf_consistent(struct quotient_filter *qf)
{
  assert(qf->qf_qbits);
  assert(qf->qf_rbits);
  assert(qf->qf_qbits + qf->qf_rbits <= 64);
  assert(qf->qf_elem_bits == (qf->qf_rbits + 3));
  assert(qf->qf_table);

  uint64_t idx;
  uint64_t start;
  uint64_t size = qf->qf_max_size;
  assert(qf->qf_entries <= size);
  uint64_t last_run_elt;
  uint64_t visited = 0;

  if (qf->qf_entries == 0) {
    for (start = 0; start < size; ++start) {
      assert(get_elem(qf, start) == 0);
    }
    return;
  }

  for (start = 0; start < size; ++start) {
    if (is_cluster_start(get_elem(qf, start))) {
      break;
    }
  }

  assert(start < size);

  idx = start;
  do {
    uint64_t elt = get_elem(qf, idx);

    /* Make sure there are no dirty entries. */
    if (is_empty_element(elt)) {
      assert(get_remainder(elt) == 0);
    }

    /* Check for invalid metadata bits. */
    if (is_continuation(elt)) {
      assert(is_shifted(elt));

      /* Check that this is actually a continuation. */
      uint64_t prev = get_elem(qf, decr(qf, idx));
      assert(!is_empty_element(prev));
    }

    /* Check that remainders within runs are sorted. */
    if (!is_empty_element(elt)) {
      uint64_t rem = get_remainder(elt);
      if (is_continuation(elt)) {
        assert(rem > last_run_elt);
      }
      last_run_elt = rem;
      ++visited;
    }

    idx = incr(qf, idx);
  } while (idx != start);

  assert(qf->qf_entries == visited);
}

/* Generate a random 64-bit hash. If @clrhigh, clear the high (64-p) bits. */
static uint64_t genhash(struct quotient_filter *qf, bool clrhigh,
    set<uint64_t> &keys)
{
	
  uint64_t hash;
  uint64_t mask = clrhigh ? LOW_MASK(qf->qf_qbits + qf->qf_rbits) : ~0ULL;
  uint64_t size = qf->qf_max_size;
  
  /* If the QF is overloaded, use a linear scan to find an unused hash. */
  if (keys.size() > (3 * (size / 4))) {
    uint64_t probe;
    uint64_t start = rand64() & qf->qf_index_mask;
    for (probe = incr(qf, start); probe != start; probe = incr(qf, probe)) {
      if (is_empty_element(get_elem(qf, probe))) {
        uint64_t hi = clrhigh ? 0 : (rand64() & ~mask);
        hash = hi | (probe << qf->qf_rbits) | (rand64() & qf->qf_rmask);
        if (!keys.count(hash)) {
          return hash;
        }
      }
    }
  }

  /* Find a random unused hash. */
  do {
    hash = rand64() & mask;
  } while (keys.count(hash));  //这个是计数的，如果计数为0，就说明这个hash没有用过，
  return hash;
}

/* Insert a random p-bit hash into the QF. */
static void ht_put(struct quotient_filter *qf, set<uint64_t> &keys)
{
  uint64_t hash = genhash(qf, true, keys);
  assert(qf_insert(qf, hash));
  keys.insert(hash);
}

/* Remove a hash from the filter. */
static void ht_del(struct quotient_filter *qf, set<uint64_t> &keys)
{
  set<uint64_t>::iterator it;
  uint64_t idx = rand64() % keys.size();
  for (it = keys.begin(); it != keys.end() && idx; ++it, --idx);
  uint64_t hash = *it;
  assert(qf_remove(qf, hash));
  assert(!qf_may_contain(qf, hash));
  keys.erase(hash);
}

/* Check that a set of keys are in the QF. */
static void ht_check(struct quotient_filter *qf, set<uint64_t> &keys)
{
  qf_consistent(qf);
  set<uint64_t>::iterator it;
  for (it = keys.begin(); it != keys.end(); ++it) {
    uint64_t hash = *it;
    assert(qf_may_contain(qf, hash));
  }
}

static void qf_test(struct quotient_filter *qf)
{
	clock_t start,finish;
  /* Basic get/set tests. */
  uint64_t idx;
  uint64_t size = qf->qf_max_size;
  for (idx = 0; idx < size; ++idx) {
    assert(get_elem(qf, idx) == 0);
    set_elem(qf, idx, idx & qf->qf_elem_mask);
  }
  for (idx = 0; idx < size; ++idx) {
    assert(get_elem(qf, idx) == (idx & qf->qf_elem_mask));
  }
  qf_clear(qf);

  /* Random get/set tests. */
  vector<uint64_t> elements(size, 0);
  for (idx = 0; idx < size; ++idx) {
    uint64_t slot = rand64() % size;
    uint64_t hash = rand64();
    set_elem(qf, slot, hash & qf->qf_elem_mask);
    elements[slot] = hash & qf->qf_elem_mask;
  }
  for (idx = 0; idx < elements.size(); ++idx) {
    assert(get_elem(qf, idx) == elements[idx]);
  }
  qf_clear(qf);
  start=clock();
  /* Check: forall x, insert(x) => may-contain(x). */
  set<uint64_t> keys;
  for (idx = 0; idx < size; ++idx) {
    uint64_t elt = genhash(qf, false, keys);
    assert(qf_insert(qf, elt));
    keys.insert(elt);
  }
  finish=clock();
  double duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
  printf( "\n add time is %lf s\n", duration );
  ht_check(qf, keys);
  keys.clear();
  qf_clear(qf);

  /* Check that the QF works like a hash set when all keys are p-bit values. */
  for (idx = 0; idx < ROUNDS_MAX; ++idx) {
    while (qf->qf_entries < size) {
      ht_put(qf, keys);
    }

    while (qf->qf_entries > (size / 2)) {
      ht_del(qf, keys);
    }
    ht_check(qf, keys);

    struct qf_iterator qfi;
    qfi_start(qf, &qfi);
    while (!qfi_done(qf, &qfi)) {
      uint64_t hash = qfi_next(qf, &qfi);
      assert(keys.count(hash));
    }
  }
}

/* Fill up the QF (at least partially). */
static void random_fill(struct quotient_filter *qf)
{
  set<uint64_t> keys;
  uint64_t elts = ((uint64_t) rand()) % qf->qf_max_size;
  while (elts) {
    ht_put(qf, keys);
    --elts;
  }
  qf_consistent(qf);
}

/* Check if @lhs is a subset of @rhs. */
static void subsetof(struct quotient_filter *lhs, struct quotient_filter *rhs)
{
  struct qf_iterator qfi;
  qfi_start(lhs, &qfi);
  while (!qfi_done(lhs, &qfi)) {
    uint64_t hash = qfi_next(lhs, &qfi);
    assert(qf_may_contain(rhs, hash));
  }
}

/* Check if @qf contains both @qf1 and @qf2. */
static void supersetof(struct quotient_filter *qf, struct quotient_filter *qf1,
    struct quotient_filter *qf2)
{
  struct qf_iterator qfi;
  qfi_start(qf, &qfi);
  while (!qfi_done(qf, &qfi)) {
    uint64_t hash = qfi_next(qf, &qfi);
    assert(qf_may_contain(qf1, hash) || qf_may_contain(qf2, hash));
  }
}
void Hash(struct quotient_filter *qf,const std::string key)
{
    int capacity = 1000000000;
    double error_rate =0.5;
    int nfuncs = (int) ceil(log(1 / error_rate) / log(2));
    int counts_per_func = (int) ceil(capacity * fabs(log(error_rate)) / (nfuncs * pow(log(2), 2)));
    unsigned int i;
    uint64_t checksum[4];
    MurmurHash3_x64_128(key.c_str(), key.length(), SALT_CONSTANT, checksum);
    uint64_t h1 = checksum[0];
    uint64_t h2 = checksum[1];    
    for (i = 0; i < 1; i++) {			//进行了nfuncs个hash
        qf->hashes[i] = ((h1 + i * h2) % counts_per_func + i * counts_per_func);
        
		/*cout << hashes[i]<<" ";*/
    }
}
static void qf_bench()
{
  struct quotient_filter qf;
  const uint32_t q_large = 28;
  const uint32_t q_small = 16;
  const uint32_t nlookups = 1500000;
  clock_t start,finish,totaltime=0;
  

  uint64_t len = 10000;
  /* Test random inserts + lookups. */
  uint32_t ninserts = (3*(1<<q_large)/4);
  printf("QF Testing %d items \n",len );  //%u是无符号十进制数
  //fflush(stdout);
  qf_init(&qf, q_large,6);
   
char c[10];



set<uint64_t> keys;
set<uint64_t> keys2;

int sum=0;
start = clock();
for(uint64_t i=0;i<len;i++)
{
   sprintf(c,"%llu",i);
   Hash(&qf,c);
   assert(qf_insert(&qf,qf.hashes[0]));	
  //assert(qf_insert(&qf,elem));	
  // keys.insert(qf.hashes[0]);
}
finish=clock();
 double duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
 printf("fsd");
printf( "add time is %lf s\n", duration );


totaltime = totaltime + (finish - start);

sum=0;
start=clock();
for (uint64_t i = 0; i<len; i++)
{
	sprintf(c, "%llu", i);
	Hash(&qf, c);
	if (qf_may_contain(&qf, qf.hashes[0])){
		sum += 1;
	}
}
finish=clock();
duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
printf("");
printf( "query time is %lf s\n", duration );


totaltime = totaltime + (finish-start);

sum=0;
for(uint64_t i=len;i<2*len;i++)
{
   sprintf(c,"%lld",i);
   Hash(&qf,c);
   if(qf_may_contain(&qf, qf.hashes[0]))
{   sum+=1;}	
}

start=clock();
  for (uint64_t i = 0; i < len; ++i) {
	  sprintf(c, "%lld", i);
	  Hash(&qf, c);
	  assert(qf_remove(&qf, qf.hashes[0]));
  }
  finish=clock();
  duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
  printf("");
  printf( "delete time is %lf s\n", duration );
  totaltime = totaltime + (finish - start);

  printf("False in is %d  \n", sum);
  printf("The totalTime is: %lf s", (double)totaltime / CLOCKS_PER_SEC) ;		//输出时间

  system("pause");
/*start=clock();
  for (uint64_t i = 0; i < 100000; ++i) {
    if(qf_remove(&qf, i))
        {   }
  }
  finish=clock();
  duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
  printf( "\n delete time is %lf s\n", duration );
  start=clock();
  while (qf.qf_entries < ninserts) {
    assert(qf_insert(&qf, (uint64_t) rand()));
    
    if (qf.qf_entries % 100000 == 0) {
      printf(".");
      fflush(stdout);
    }

  }
  finish=clock();
   duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
  printf( "\n add time is %lf s\n", duration );

  start=clock();
  for (uint32_t i = 0; i < nlookups; ++i) {
  // printf("%llu\n",(uint64_t)rand());
    if(qf_may_contain(&qf, (uint64_t) rand()))
{   }
  }
  finish=clock();
  duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
  printf( "\n query time is %lf s\n", duration );
  gettimeofday(&tv2, NULL);
  sec = tv2.tv_sec - tv1.tv_sec;
  printf(" done (%llu seconds).\n", sec);
  fflush(stdout);
  qf_destroy(&qf);

  /* Create a large cluster. Test random lookups. 
  qf_init(&qf, q_small, 1);
  printf("Testing %u contiguous inserts and %u lookups", 1 << q_small,
      nlookups);
  fflush(stdout);
  gettimeofday(&tv1, NULL);
  for (uint64_t quot = 0; quot < (1 << (q_small - 1)); ++quot) {
    uint64_t hash = quot << 1;
    assert(qf_insert(&qf, hash));
    assert(qf_insert(&qf, hash | 1));
    if (quot % 2000 == 0) {
      printf(".");
      fflush(stdout);
    }
  }
  for (uint32_t i = 0; i < nlookups; ++i) {
    qf_may_contain(&qf, (uint64_t) rand());
    if (i % 50000 == 0) {
      printf(".");
      fflush(stdout);
    }
  }
  gettimeofday(&tv2, NULL);
  sec = tv2.tv_sec - tv1.tv_sec;
  printf(" done (%llu seconds).\n", sec);
  fflush(stdout);
  qf_destroy(&qf);*/
}

int main()
{
  srand(0);

#if QBENCH
  qf_bench();
#else
 /* for (uint32_t q = 1; q <= Q_MAX; ++q) {
    printf("Starting rounds for qf_test::q=%u\n", q);

#pragma omp parallel for  //并行编程 表示接下来的循环多线程执行，每次循环之间不能有关系
    for (uint32_t r = 6; r <= R_MAX; ++r) {
      struct quotient_filter qf;
      if (!qf_init(&qf, q, r)) {  //初始化
        fail(&qf, "init-1");
      }
      qf_test(&qf);
      qf_destroy(&qf);
    }
  }
*/

clock_t start,finish;
  
struct quotient_filter qf;
struct quotient_filter qf1, qf2,qf3,qf4,qf5,qf6,qf7,qf8;
uint32_t q1=24,q2=24,q3=24;
uint32_t r1=6,r2=6,r3=6;


qf_init(&qf1, q1, r1);
qf_init(&qf2, q2, r2);
qf_init(&qf4, q3, r2);
qf_init(&qf3, q3, r3);
qf_init(&qf5, q3, r3);
qf_init(&qf7, q3, r3);
int sum = 0,len=50000;
char c[10];
for (uint64_t i = 0; i<len; i++)
{
	sprintf(c, "%llu", i);
	Hash(&qf1, c);
	assert(qf_insert(&qf1, qf1.hashes[0]));
}
for (uint64_t i = 0; i<len; i++)
{
	sprintf(c, "%llu", i);
	Hash(&qf2, c);
	assert(qf_insert(&qf2, qf2.hashes[0]));
}
for (uint64_t i = 0; i<len; i++)
{
	sprintf(c, "%llu", i);
	Hash(&qf3, c);
	assert(qf_insert(&qf3, qf3.hashes[0]));
}
for (uint64_t i = 0; i<len; i++)
{
	sprintf(c, "%llu", i);
	Hash(&qf5, c);
	assert(qf_insert(&qf5, qf5.hashes[0]));
}

printf("dsf");

//random_fill(&qf1);
//random_fill(&qf2);
//random_fill(&qf3);
//random_fill(&qf5);
//random_fill(&qf7);

start=clock();
qf_merge(&qf1, &qf2, &qf4);
qf_merge(&qf4, &qf3, &qf);

qf_merge(&qf, &qf5, &qf6);

qf_merge(&qf6, &qf7, &qf8);


finish=clock();
double duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
printf("");
printf( "\n merge time is %lf s\n", duration );

system("pause");
/*

clock_t start,finish;
 
  for (uint32_t q1 = 1; q1 <= Q_MAX; ++q1) {
    for (uint32_t r1 = 1; r1 <= R_MAX; ++r1) {
      for (uint32_t q2 = 1; q2 <= Q_MAX; ++q2) {
	start=clock();
        printf("Starting rounds for qf_merge::q1=%u,r1=%u,q2=%u  ", q1,r1, q2);


        for (uint32_t r2 = 1; r2 <= R_MAX; ++r2) {
	
          struct quotient_filter qf;
          struct quotient_filter qf1, qf2;
          if (!qf_init(&qf1, q1, r1) || !qf_init(&qf2, q2, r2)) {
            fail(&qf1, "init-2");
          }

          random_fill(&qf1);
          random_fill(&qf2);
          assert(qf_merge(&qf1, &qf2, &qf));
          qf_consistent(&qf);
          subsetof(&qf1, &qf);
          subsetof(&qf2, &qf);
          supersetof(&qf, &qf1, &qf2);
          qf_destroy(&qf1);
          qf_destroy(&qf2);
          qf_destroy(&qf);
        }
	finish=clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;//转换浮点型
        printf( "time is %lf s\n", duration );
      }
	
    }
  }*/
#endif /* QBENCH */

  puts("[PASSED] qf tests");
  return 0;
}
