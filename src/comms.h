#ifndef _COMMS_H
#define _COMMS_H 1

typedef void *shmem_handle_t;

extern void __comms_init(void);
extern void __comms_shutdown(int status);

extern int __comms_mynode(void);
extern int __comms_nodes(void);

extern void   __symmetric_memory_init(void);
extern void   __symmetric_memory_finalize(void);
extern void * __symmetric_var_base(int pe);
extern int    __symmetric_var_in_range(void *addr, int pe);

extern void __comms_barrier_all(void);

extern void __comms_poll(void);

#endif /* _COMMS_H */
