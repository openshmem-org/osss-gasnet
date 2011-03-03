#ifndef _PING_H
#define _PING_H 1

/*
 * initialize the ping subsystem
 */
extern void __shmem_ping_init(void);

/*
 * set the amount of time to wait
 */
extern void __shmem_set_ping_timeout(double secs);

/*
 * set & clear alarms
 */
extern void __shmem_ping_set_alarm(void);
extern void __shmem_ping_clear_alarm(void);

#endif /* _PING_H */
