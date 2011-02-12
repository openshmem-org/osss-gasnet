#ifndef _PING_H
#define _PING_H 1

extern void __shmem_set_ping_timeout(int secs);

extern void __shmem_ping_init(void);

extern void __ping_set_alarm(void);
extern void __ping_clear_alarm(void);

#endif /* _PING_H */
