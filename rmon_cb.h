/*
 * rmon_public.h: Handle all the callback functions which are assigned in 
 *                application based on platform
 */

#ifndef __RMON_CB_H__
#define __RMON_CB_H__

extern void *(* rmon_alloc_memory)(int); 
extern void (* rmon_free_memory)(int, void *);

typedef void (*rmon_timer_cb)(void *, void *);
extern void (* rmon_timer_create)(int, int, rmon_timer_cb, void *);
extern void (* rmon_timer_delete)(int);

extern int (* rmon_get_sys_uptime)();
extern void (* do_rmon_trace)(int, int, char *, ...);

#endif /* __RMON_CB_H__ */

