#ifndef _SERVICE_H
#define _SERVICE_H 1

extern void __shmem_service_thread_init(void);
extern void __shmem_service_thread_finalize(void);

typedef enum {
  SERVICE_FINISH=0,
  SERVICE_POLL,
  SERVICE_FENCE,
  SERVICE_SPINUP,
} poll_mode_t;

extern void __shmem_service_set_mode(poll_mode_t m);

#endif /* _SERVICE_H */
