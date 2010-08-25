#ifndef _COMMS_H
#define _COMMS_H 1

typedef void *shmem_handle_t;

extern void __comms_init(void);
extern void __comms_shutdown(int status);

extern void __comms_set_waitmode(int mode);

#define SHMEM_COMMS_SPINBLOCK 0
#define SHMEM_COMMS_SPIN      1
#define SHMEM_COMMS_BLOCK     2

extern int __comms_mynode(void);
extern int __comms_nodes(void);

extern void   __symmetric_memory_init(void);
extern void   __symmetric_memory_finalize(void);
extern void * __symmetric_var_base(int pe);
extern int    __symmetric_var_in_range(void *addr, int pe);
extern void * __symmetric_var_offset(void *dest, int pe);

extern void __comms_barrier_all(void);

extern void __comms_poll(void);

extern long __comms_request(void *target, long value, int pe);

#endif /* _COMMS_H */
