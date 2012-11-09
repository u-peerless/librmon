/*
 * rmon_disp.c : 
 */
#include "stdint.h"
#include "linux/types.h"
#include "stdlib.h"
#include "assert.h"
#include "atavl.h"
#include "rmon_defs.h"
#include "rmon_cb.h"


size_t
rmon_get_entry_size_by_entry_type (int type)
{
    switch (type) {
    case RMON_ETHERSTAT_ENTRY:
        return (sizeof(etherstat_entry_t));
    case RMON_ETHERSTAT_START_ENTRY:
        return (sizeof(rmon_etherstat_start_entry_t));
    case RMON_IFSTAT_ENTRY:
        return (sizeof(rmon_ifstat_entry_t));
    case RMON_IFSTAT_START_ENTRY:
        return (sizeof(rmon_ifstat_start_entry_t));
    case RMON_IFSTAT_DATA_ENTRY:
        return (sizeof(rmon_ifstat_data_t));
    case RMON_HISTORY_DATA_ENTRY:
        return (sizeof(history_data_t));
    case RMON_EVENT_ENTRY:
        return (sizeof(event_entry_t));
    case RMON_LOG_ENTRY:
        return (sizeof(log_entry_t));
    case RMON_ALARM_ENTRY:
        return (sizeof(alarm_entry_t));
    case RMON_HISTORY_ENTRY:
        return (sizeof(history_table_t));
    case RMON_HISTORY_CTRL_ENTRY:
        return (sizeof(rmon_hist_ctrl_data_t));
    case RMON_HISTORY_DIRTY_ENTRY:
        return (sizeof(history_table_t));
    case RMON_CONFIG_SYNC_ENTRY:
        return (sizeof(rmon_config_sync_t));
    default:
        return 0;
    }
}

void *
fn_rmon_alloc_memory (int type)
{
    size_t size = rmon_get_entry_size_by_entry_type(type);
    return calloc(1, size);
}

void
fn_rmon_free_memory (int dummy, void *ptr)
{
    free(ptr);
}

void
rmon_init_dispatcher ()
{
    if (!rmon_disp_id) {
        rmon_disp_id = DispatcherCreate();
        assert(rmon_disp_id);
    }
}

void
fn_rmon_timer_create (int type, int sec, rmon_timer_cb func, void *data)
{
}

/* This returns sysUpTime in Centi Seconds unit */
int
fn_rmon_get_sys_uptime ()
{
}

void
fn_rmon_timer_delete (int type)
{
}

void
fn_do_rmon_trace (int flag, int val, int level, int lval, char *fmt,
                  ...)
{
}


