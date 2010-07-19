/* TODO: doesn't work, can't work, was an idea though... */

#include <stdio.h>
#include <pthread.h>

#include "shmem.h"
#include "state.h"

#define SHMEM_THREAD_ARGS_TYPE(Name, Type) \
struct __thread_args_##Name { \
  Type *ivar; \
  int cmp; \
  Type cmp_value; \
};
  
SHMEM_THREAD_ARGS_TYPE(short, short)
SHMEM_THREAD_ARGS_TYPE(int, int)
SHMEM_THREAD_ARGS_TYPE(long, long)
SHMEM_THREAD_ARGS_TYPE(longlong, long long)

#define SHMEM_WAIT_LOOP_FRAGMENT(Type, Var, Op)		\
while ( *((volatile Type *) Var) Op cmp_value) \
{ \
  fprintf(stderr, "SHMEM(PE %d): in wait...\n", __state.mype); \
  sched_yield(); \
}
  
#define SHMEM_TYPE_WAIT_UNTIL_LAUNCH(Name, Type)				\
static void *									\
__shmem_##Name##_wait_until_launch(void *args)		\
{									\
  struct __thread_args_##Name *ta = (struct __thread_args_##Name *)args; \
  Type *ivar = ta->ivar;                                                 \
  int cmp = ta->cmp;                                                 \
  Type cmp_value = ta->cmp_value;                                       \
  switch (cmp) {							\
  case SHMEM_CMP_EQ:							\
    SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, ==);				\
    break;								\
  case SHMEM_CMP_NE:							\
    SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, !=);				\
    break;								\
  case SHMEM_CMP_GT:							\
    SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, >);				\
    break;								\
  case SHMEM_CMP_LE:							\
    SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, <=);				\
    break;								\
  case SHMEM_CMP_LT:							\
    SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, <);				\
    break;								\
  case SHMEM_CMP_GE:							\
    SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, >=);				\
    break;								\
  default:								\
    /* error handler goes here */					\
    break;								\
  }									\
}

SHMEM_TYPE_WAIT_UNTIL_LAUNCH(short, short)
SHMEM_TYPE_WAIT_UNTIL_LAUNCH(int, int)
SHMEM_TYPE_WAIT_UNTIL_LAUNCH(long, long)
SHMEM_TYPE_WAIT_UNTIL_LAUNCH(longlong, long long)

  
#define SHMEM_TYPE_WAIT_UNTIL(Name, Type)				\
void 							\
shmem_##Name##_wait_until(Type *ivar, int cmp, Type cmp_value)		\
{									\
  pthread_t t;                                                          \
  struct __thread_args_##Name ta;                                   \
  ta.ivar = ivar;                                   \
  ta.cmp = cmp;                                   \
  ta.cmp_value = cmp_value;                                \
  pthread_create(&t, NULL, __shmem_##Name##_wait_until_launch, (void *)&ta); \
  pthread_join(t, NULL); \
}

SHMEM_TYPE_WAIT_UNTIL(short, short)
SHMEM_TYPE_WAIT_UNTIL(int, int)
SHMEM_TYPE_WAIT_UNTIL(long, long)
SHMEM_TYPE_WAIT_UNTIL(longlong, long long)

_Pragma("weak shmem_wait_until=shmem_long_wait_until")

#define SHMEM_TYPE_WAIT(Name, Type)			\
void							\
shmem_##Name##_wait(Type *ivar, Type cmp_value)	        \
{							\
  shmem_##Name##_wait_until(ivar, SHMEM_CMP_EQ, cmp_value); \
}

SHMEM_TYPE_WAIT(short, short)
SHMEM_TYPE_WAIT(int, int)
SHMEM_TYPE_WAIT(long, long)
SHMEM_TYPE_WAIT(longlong, long long)

_Pragma("weak shmem_wait=shmem_long_wait")
