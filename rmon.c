/*
 * rmon.c
 */
#include "atavl.h"
#include "rmon_defs.h"
#include "rmon_cb.h"

atavl_tree_t history_table_tree;
atavl_tree_t rmon_ifstat_data_tree;
atavl_tree_t rmon_lif_tree;
atavl_tree_t rmon_event_tree;
atavl_tree_t rmon_alarm_tree;
atavl_tree_t rmon_log_tree;
atavl_tree_t rmon_log_time_tree;
atavl_tree_t rmon_log_event_tree;
atavl_tree_t rmon_etherstat_tree;
atavl_tree_t rmon_etherstat_start_tree;
atavl_tree_t rmon_ifstat_start_tree;
atavl_tree_t history_dirty_entry_tree;

void *(* rmon_alloc_memory)(int) = NULL;
void (* rmon_free_memory)(int, void *) = NULL;
void (* rmon_timer_create)(int, int, rmon_timer_cb, void *) = NULL;
void (* rmon_timer_delete)(int) = NULL;
int (* rmon_get_sys_uptime)();
void (* do_rmon_trace)(int, int, char *, ...);

int
rmon_ifstat_node_cmp (const void *ptr1, const void *ptr2)
{
    rmon_ifstat_data_t *p1 = NULL;
    rmon_ifstat_data_t *p2 = NULL;

    p1 = (rmon_ifstat_data_t *)ptr1;
    p2 = (rmon_ifstat_data_t *)ptr2;
    return (p1->if_index - p2->if_index);
}

int
rmon_ifstat_start_node_cmp (const void *ptr1, const void *ptr2)
{
    rmon_ifstat_start_entry_t *p1 = NULL;
    rmon_ifstat_start_entry_t *p2 = NULL;

    p1 = (rmon_ifstat_start_entry_t *)ptr1;
    p2 = (rmon_ifstat_start_entry_t *)ptr2;
    return (p1->hist_index - p2->hist_index);
}

int
rmon_lif_node_cmp (const void *ptr1, const void *ptr2)
{
    rmon_lif_data_t *p1 = NULL;
    rmon_lif_data_t *p2 = NULL;

    p1 = (rmon_lif_data_t *)ptr1;
    p2 = (rmon_lif_data_t *)ptr2;
    return (p1->if_index - p2->if_index);
}

int
rmon_alarm_node_cmp (const void *ptr1, const void *ptr2)
{
    alarm_entry_t *p1 = NULL;
    alarm_entry_t *p2 = NULL;

    p1 = (alarm_entry_t *)ptr1;
    p2 = (alarm_entry_t *)ptr2;
    return (p1->alarm_index - p2->alarm_index);
}

int
rmon_event_node_cmp (const void *ptr1, const void *ptr2)
{
    event_entry_t *p1 = NULL;
    event_entry_t *p2 = NULL;

    p1 = (event_entry_t *)ptr1;
    p2 = (event_entry_t *)ptr2;
    return (p1->event_index - p2->event_index);
}

int
rmon_log_node_cmp (const void *ptr1, const void *ptr2)
{
    log_entry_t *p1 = NULL;
    log_entry_t *p2 = NULL;

    p1 = (log_entry_t *)ptr1;
    p2 = (log_entry_t *)ptr2;
    if (p1->event_index != p2->event_index) {
        return (p1->event_index - p2->event_index);
    }
    if (p1->log_index != p2->log_index) {
        return (p1->log_index - p2->log_index);
    }
    return 0;
}

int
rmon_log_time_node_cmp (const void *ptr1, const void *ptr2)
{
    log_entry_t *p1 = NULL;
    log_entry_t *p2 = NULL;

    p1 = (log_entry_t *)ptr1;
    p2 = (log_entry_t *)ptr2;

    if (p1->log_time != p2->log_time) {
        return (p1->log_time - p2->log_time);
    }
    if (p1->event_index != p2->event_index) {
        return (p1->event_index - p2->event_index);
    }
    if (p1->log_index != p2->log_index) {
        return (p1->log_index - p2->log_index);
    }
    return 0;
}

int
rmon_log_event_node_cmp (const void *ptr1, const void *ptr2)
{
    log_entry_t *p1 = NULL;
    log_entry_t *p2 = NULL;

    p1 = (log_entry_t *)ptr1;
    p2 = (log_entry_t *)ptr2;

    if (p1->event_index != p2->event_index) {
        return (p1->event_index - p2->event_index);
    }
    if (p1->log_time != p2->log_time) {
        return (p1->log_time - p2->log_time);
    }
    if (p1->log_index != p2->log_index) {
        return (p1->log_index - p2->log_index);
    }
    return 0;
}

int
history_node_cmp (const void *ptr1, const void *ptr2)
{
    history_table_t *p1 = NULL;
    history_table_t *p2 = NULL;

    p1 = (history_table_t *)ptr1;
    p2 = (history_table_t *)ptr2;
    return (p1->history_index - p2->history_index);
}

int
dirty_node_cmp (const void *ptr1, const void *ptr2)
{
    history_table_t *p1 = NULL;
    history_table_t *p2 = NULL;

    p1 = (history_table_t *)ptr1;
    p2 = (history_table_t *)ptr2;
    return (p1->history_index - p2->history_index);
}

int
rmon_etherstat_node_cmp (const void *ptr1, const void *ptr2)
{
    etherstat_entry_t *p1 = NULL;
    etherstat_entry_t *p2 = NULL;

    p1 = (etherstat_entry_t *)ptr1;
    p2 = (etherstat_entry_t *)ptr2;

    return (p1->stat_index - p2->stat_index);
}

int
rmon_etherstat_start_node_cmp (const void *ptr1, const void *ptr2)
{
    rmon_etherstat_start_entry_t *p1 = NULL;
    rmon_etherstat_start_entry_t *p2 = NULL;

    p1 = (rmon_etherstat_start_entry_t *)ptr1;
    p2 = (rmon_etherstat_start_entry_t *)ptr2;

    return (p1->stat_index - p2->stat_index);
}

void
rmon_history_init_tree ()
{
    atavl_tree_init(&history_table_tree, history_node_cmp,
                    offsetof(history_table_t, history_node),
                    ATAVL_NO_DUP);
    return;
}

void
rmon_init_dirty_tree ()
{
    atavl_tree_init(&history_dirty_entry_tree, dirty_node_cmp,
                    offsetof(history_table_t, dirty_node),
                    ATAVL_NO_DUP);
    return;
}

void
rmon_ifstat_init_tree ()
{
    atavl_tree_init(&rmon_ifstat_data_tree, rmon_ifstat_node_cmp,
                    offsetof(rmon_ifstat_data_t, ifstat_node),
                    ATAVL_NO_DUP);
    return;
}

void
rmon_lif_init_tree ()
{
    atavl_tree_init(&rmon_lif_tree, rmon_lif_node_cmp,
                    offsetof(rmon_lif_data_t, lif_node),
                    ATAVL_NO_DUP);
    return;
}

void
rmon_alarm_init_tree ()
{
    atavl_tree_init(&rmon_alarm_tree, rmon_alarm_node_cmp,
                    offsetof(alarm_entry_t, alarm_node),
                    ATAVL_NO_DUP);
    return;
}

void
rmon_event_init_tree ()
{
    atavl_tree_init(&rmon_event_tree, rmon_event_node_cmp,
                    offsetof(event_entry_t, event_node),
                    ATAVL_NO_DUP);
    return;
}


void
rmon_log_init_tree ()
{
    atavl_tree_init(&rmon_log_tree, rmon_log_node_cmp,
                    offsetof(log_entry_t, log_node),
                    ATAVL_NO_DUP);
    return;
}

void
rmon_log_time_tree_init ()
{
    atavl_tree_init(&rmon_log_time_tree, rmon_log_time_node_cmp,
                    offsetof(log_entry_t, log_time_node),
                    ATAVL_NO_DUP);
    return;
} 

void
rmon_log_event_tree_init ()
{
    atavl_tree_init(&rmon_log_event_tree, rmon_log_event_node_cmp,
                    offsetof(log_entry_t, log_time_event_node),
                    ATAVL_NO_DUP);
    return;
} 

void
rmon_etherstat_init_tree ()
{
    atavl_tree_init(&rmon_etherstat_tree, rmon_etherstat_node_cmp,
                    offsetof(etherstat_entry_t, etherstat_node),
                    ATAVL_NO_DUP);
    return;
}

void
rmon_etherstat_start_init_tree ()
{
    atavl_tree_init(&rmon_etherstat_start_tree, rmon_etherstat_start_node_cmp,
                    offsetof(rmon_etherstat_start_entry_t, etherstat_start_node),
                    ATAVL_NO_DUP);
    return;
}

void
rmon_ifstat_start_init_tree ()
{
    atavl_tree_init(&rmon_ifstat_start_tree, rmon_ifstat_start_node_cmp,
                    offsetof(rmon_ifstat_start_entry_t, ifstat_start_node),
                    ATAVL_NO_DUP);
    return;
}

history_table_t *
history_get_first_entry ()
{
    return (history_table_t *)atavl_first(&history_table_tree);
}

history_table_t *
history_get_entry (int history_index)
{
    history_table_t hist_data;

    hist_data.history_index = history_index;
    return (history_table_t *)atavl_find(&history_table_tree, &hist_data);
}

history_table_t *
history_get_next_entry (int history_index)
{
    history_table_t hist_data;

    hist_data.history_index = history_index;
    return (history_table_t *)atavl_find_next(&history_table_tree, &hist_data);
}

log_entry_t *
rmon_get_first_log_entry ()
{
    return (log_entry_t *)atavl_first(&rmon_log_tree);
}

log_entry_t *
rmon_get_log_entry (int log_index, int event_index)
{
    log_entry_t log_data;

    log_data.log_index = log_index;
    log_data.event_index = event_index;
    return (log_entry_t *)atavl_find(&rmon_log_tree, &log_data);
}

log_entry_t *
rmon_get_next_log_entry (int log_index, int event_index)
{
    log_entry_t log_data;

    log_data.log_index = log_index;
    log_data.event_index = event_index;
    return (log_entry_t *)atavl_find_next(&rmon_log_tree, &log_data);
}

log_entry_t *
rmon_get_prev_log_entry (int log_index, int event_index)
{
    log_entry_t log_data;

    log_data.log_index = log_index;
    log_data.event_index = event_index;
    return (log_entry_t *)atavl_prev(&rmon_log_tree, &log_data);
}

/* This API returns the next entry in log table for this event id */
log_entry_t *
rmon_get_next_log_entry_by_event_id (int event_index, int log_index)
{
    log_entry_t  log_data;
    log_entry_t *log_entry = NULL;

    log_data.log_index = log_index;
    log_data.event_index = event_index;
    log_entry = (log_entry_t *)atavl_find_next(&rmon_log_tree, &log_data);

    if (NULL == log_entry) {
        return NULL;
    }

    /* Check if event id matches */
    if (event_index == log_entry->event_index) {
        /* Same event id */
        return log_entry;
    } else {
        return NULL;
    }
}

/* This API returns the next entry in log table for this event id */
log_entry_t *
rmon_get_prev_log_entry_by_event_id (int event_index, int log_index)
{
    log_entry_t  log_data;
    log_entry_t *log_entry = NULL;

    log_data.log_index = log_index;
    log_data.event_index = event_index;
    log_entry = (log_entry_t *)atavl_prev(&rmon_log_tree, &log_data);

    if (NULL == log_entry) {
        return NULL;
    }

    /* Check if event id matches */
    if (event_index == log_entry->event_index) {
        /* Same event id */
        return log_entry;
    } else {
        return NULL;
    }
}

log_entry_t *
rmon_get_next_log_entry_by_event (log_entry_t *log_ptr)
{
    log_entry_t *log_entry = NULL;

    log_entry = (log_entry_t *)atavl_find_next(&rmon_log_tree, log_ptr);

    if (NULL == log_entry) {
        return NULL;
    }

    if (log_ptr->event_index == log_entry->event_index) {
        return log_entry;
    } else {
        return NULL;
    }
}

log_entry_t *
rmon_get_prev_log_entry_by_event (log_entry_t *log_ptr)
{
    log_entry_t *log_entry = NULL;

    log_entry = (log_entry_t *)atavl_prev(&rmon_log_tree, log_ptr);

    if (NULL == log_entry) {
        return NULL;
    }

    if (log_ptr->event_index == log_entry->event_index) {
        return log_entry;
    } else {
        return NULL;
    }
}

log_entry_t *
rmon_prev_log_event_entry (int event_index, int log_time, int log_index)
{
    log_entry_t  log_data;
    log_entry_t *log_ptr = NULL;

    log_data.log_time = log_time;
    log_data.event_index = event_index;
    log_data.log_index = log_index;

    return (log_entry_t *)atavl_prev(&rmon_log_event_tree, &log_data);
}

log_entry_t *
rmon_get_first_log_time_entry ()
{
    return (log_entry_t *)atavl_first(&rmon_log_time_tree);
}

log_entry_t *
rmon_get_last_log_time_entry ()
{
    return (log_entry_t *)atavl_last(&rmon_log_time_tree);
}

log_entry_t *
rmon_get_log_time_entry (int log_time, int event_index, int log_index)
{
    log_entry_t log_data;

    log_data.log_time = log_time;
    log_data.event_index = event_index;
    log_data.log_index = log_index;

    return (log_entry_t *)atavl_find(&rmon_log_time_tree, &log_data);
}

log_entry_t *
rmon_get_next_log_time_entry (int log_time, int event_index, int log_index)
{
    log_entry_t log_data;

    log_data.log_time = log_time;
    log_data.event_index = event_index;
    log_data.log_index = log_index;

    return (log_entry_t *)atavl_find_next(&rmon_log_time_tree, &log_data);
}

log_entry_t *
rmon_get_prev_log_time_entry (int log_time, int event_index, int log_index)
{
    log_entry_t  log_data;
    log_entry_t *log_ptr = NULL;

    log_data.log_time = log_time;
    log_data.event_index = event_index;
    log_data.log_index = log_index;

    log_ptr = (log_entry_t *)atavl_find(&rmon_log_time_tree, &log_data);
    if (NULL == log_ptr) {
        return NULL;
    }
    return (log_entry_t *)atavl_prev(&rmon_log_time_tree, log_ptr);
}

log_entry_t *
rmon_get_first_log_event_entry ()
{
    return (log_entry_t *)atavl_first(&rmon_log_event_tree);
}

log_entry_t *
rmon_get_log_event_entry (int event_index, int log_time, int log_index)
{
    log_entry_t log_data;

    log_data.log_time = log_time;
    log_data.event_index = event_index;
    log_data.log_index = log_index;

    return (log_entry_t *)atavl_find(&rmon_log_event_tree, &log_data);
}

log_entry_t *
rmon_get_next_log_event_entry (int event_index, int log_time, int log_index)
{
    log_entry_t log_data;

    log_data.log_time = log_time;
    log_data.event_index = event_index;
    log_data.log_index = log_index;

    return (log_entry_t *)atavl_find_next(&rmon_log_event_tree, &log_data);
}

/* This API returns the first entry matching event_index from
 * rmon_log_event_tree 
 */
log_entry_t *
rmon_get_first_log_entry_by_event_id (int event_index)
{
    log_entry_t log_data;

    log_data.event_index = event_index;
    log_data.log_time = 0;
    log_data.log_index = 0;
    return (log_entry_t *)atavl_find_next(&rmon_log_event_tree, &log_data);
}

event_entry_t *
rmon_get_event_entry (int index) 
{
    event_entry_t event_data;

    event_data.event_index = index;
    return (event_entry_t *)atavl_find(&rmon_event_tree, &event_data);
}

event_entry_t *
rmon_get_next_event_entry (int index)
{
    event_entry_t event_data;

    event_data.event_index = index;
    return (event_entry_t *)atavl_find_next(&rmon_event_tree, &event_data);
}

event_entry_t *
rmon_next_event_entry (event_entry_t *ptr)
{
    return (event_entry_t *)atavl_next(&rmon_event_tree, ptr, ATAVL_NO_DUP);
}

rmon_ifstat_data_t *
rmon_get_ifstat_data_entry (int if_index)
{
    rmon_ifstat_data_t ifstat_data;

    ifstat_data.if_index = if_index;
    return (rmon_ifstat_data_t *)atavl_find(&rmon_ifstat_data_tree,
                                            &ifstat_data);
}

rmon_ifstat_data_t *
rmon_get_or_new_ifstat_data_entry (int if_index)
{
    rmon_ifstat_data_t *ifstat_ptr = NULL;

    ifstat_ptr = rmon_get_ifstat_data_entry(if_index);
    if (NULL == ifstat_ptr) {
        /* Not found, create a new one */
        ifstat_ptr = rmon_alloc_memory(RMON_IFSTAT_DATA_ENTRY);
        assert(ifstat_ptr);
        ifstat_ptr->if_index = if_index;
        assert(atavl_insert(&rmon_ifstat_data_tree, ifstat_ptr) == ATAVL_OK);
    }
    return ifstat_ptr;
}

rmon_ifstat_data_t *
rmon_get_next_ifstat_data_entry (int if_index)
{
    rmon_ifstat_data_t ifstat_data;

    ifstat_data.if_index = if_index;
    return (rmon_ifstat_data_t *)atavl_find_next(&rmon_ifstat_data_tree,
                                                 &ifstat_data);
}

alarm_entry_t *
rmon_get_alarm_entry (int alarm_index)
{
    alarm_entry_t alarm_data;

    alarm_data.alarm_index = alarm_index;
    return (alarm_entry_t *)atavl_find(&rmon_alarm_tree, &alarm_data);
}

alarm_entry_t *
rmon_get_next_alarm_entry (int alarm_index)
{
    alarm_entry_t alarm_data;

    alarm_data.alarm_index = alarm_index;
    return (alarm_entry_t *)atavl_find_next(&rmon_alarm_tree, &alarm_data);
}

rmon_ifstat_start_entry_t *
rmon_get_ifstat_init_entry (int hist_index)
{
    rmon_ifstat_start_entry_t ifstat_data;

    ifstat_data.hist_index = hist_index;
    return (rmon_ifstat_start_entry_t *)atavl_find(&rmon_ifstat_start_tree,
                                                   &ifstat_data);
}

rmon_ifstat_start_entry_t *
rmon_get_next_ifstat_init_entry (int hist_index)
{
    rmon_ifstat_start_entry_t ifstat_data;

    ifstat_data.hist_index = hist_index;
    return (rmon_ifstat_start_entry_t *)atavl_find_next(&rmon_ifstat_start_tree,
                                                        &ifstat_data);
}

etherstat_entry_t *
rmon_get_etherstat_entry (int stat_index)
{
    etherstat_entry_t stat_data;

    stat_data.stat_index = stat_index;
    return (etherstat_entry_t *)atavl_find(&rmon_etherstat_tree, &stat_data);
}

etherstat_entry_t *
rmon_get_next_etherstat_entry (int stat_index)
{
    etherstat_entry_t stat_data;

    stat_data.stat_index = stat_index;
    return (etherstat_entry_t *)atavl_find_next(&rmon_etherstat_tree,
                                                &stat_data);
}

etherstat_entry_t *
rmon_next_etherstat_entry (etherstat_entry_t *ptr)
{
    return (etherstat_entry_t *)atavl_next(&rmon_etherstat_tree, ptr,
                                           ATAVL_NO_DUP);
}

void
rmon_delete_etherstat_entry (etherstat_entry_t *ptr)
{
    rmon_etherstat_start_entry_t *ether_ptr = NULL;

    ether_ptr =  rmon_get_etherstat_init_entry(ptr->stat_index);
    assert(ether_ptr);
    assert(atavl_delete(&rmon_etherstat_tree, ptr) == ATAVL_OK);
    rmon_free_memory(RMON_ETHERSTAT_ENTRY, ptr);
    assert(atavl_delete(&rmon_etherstat_start_tree, ether_ptr) == ATAVL_OK);
    rmon_free_memory(RMON_ETHERSTAT_START_ENTRY, ether_ptr);
}

void
rmon_delete_etherstat_entry_by_index (int stat_index)
{
    etherstat_entry_t *stat_ptr = NULL;

    stat_ptr = rmon_get_etherstat_entry(stat_index);
    assert(stat_ptr);
    rmon_delete_etherstat_entry(stat_ptr);
    return;
}

rmon_etherstat_start_entry_t *
rmon_get_etherstat_init_entry (int stat_index)
{
    rmon_etherstat_start_entry_t stat_data;

    stat_data.stat_index = stat_index;
    return 
        (rmon_etherstat_start_entry_t *)atavl_find(&rmon_etherstat_start_tree,
                                                   &stat_data);
}

rmon_etherstat_start_entry_t *
rmon_get_next_etherstat_init_entry (int stat_index)
{
    rmon_etherstat_start_entry_t stat_data;

    stat_data.stat_index = stat_index;
    return
        (rmon_etherstat_start_entry_t *)
         atavl_find_next(&rmon_etherstat_start_tree,
                         &stat_data);
}

history_table_t *
rmon_history_get_dirty_entry (int hist_index)
{
    history_table_t hist_data;

    memset(&hist_data, 0, sizeof(history_table_t));
    hist_data.history_index = hist_index;
    return (history_table_t *)atavl_find(&history_dirty_entry_tree, &hist_data);
}

void
rmon_delete_history_data (history_data_t *hist_data)
{
    rmon_ifstat_entry_t *ifstat_ptr = NULL;

    ifstat_ptr = hist_data->data;
    if (ifstat_ptr) {
        rmon_free_memory(RMON_IFSTAT_ENTRY, ifstat_ptr);
    }
    rmon_free_memory(RMON_HISTORY_DATA_ENTRY, hist_data);
}

char *
rmon_get_ifname_by_ifindex (int if_index)

{
    rmon_ifstat_data_t *ifstat_ptr = NULL;

    ifstat_ptr = rmon_get_ifstat_data_entry(if_index);
    if (NULL == ifstat_ptr) {
        return NULL;
    }
    return ifstat_ptr->if_name;
}

uint64_t
rmon_get_if_pkts_count (uint64_t if_stats[])
{
    return /* Below are good packets */
        (if_stats[RMON_STATS_RX_UNICAST_FRAMES] +
            if_stats[RMON_STATS_RX_MULTICAST_FRAMES] +
            if_stats[RMON_STATS_RX_BROADCAST_FRAMES] +
            /* Below are bad packets */
            if_stats[RMON_STATS_RX_ERROR_FRAMES] +
            if_stats[RMON_STATS_RX_CRC_ERROR_FRAMES] +
            if_stats[RMON_STATS_RX_RUNT_FRAMES]);
}

uint64_t
rmon_get_etherstat_pkts_count (uint64_t if_stats[])
{
    return /* Below are good packets */
        (if_stats[SYSTEM_STATS_RX_UNICAST_FRAMES] +
            if_stats[SYSTEM_STATS_RX_MULTICAST_FRAMES] +
            if_stats[SYSTEM_STATS_RX_BROADCAST_FRAMES] +
            /* Below are bad packets */
            if_stats[SYSTEM_STATS_RX_ERROR_FRAMES] +
            if_stats[SYSTEM_STATS_RX_CRC_ERROR_FRAMES] +
            if_stats[SYSTEM_STATS_RX_RUNT_FRAMES]);
}

void 
rmon_event_entry_delete (event_entry_t *event_ptr)
{
    assert(atavl_delete(&rmon_event_tree, event_ptr) == ATAVL_OK);
    rmon_free_memory(RMON_EVENT_ENTRY, event_ptr);
}

void
rmon_event_entry_db_delete ()
{
    event_entry_t *event_ptr = NULL;

    event_ptr = atavl_first(&rmon_event_tree);
    for (; event_ptr; event_ptr = atavl_first(&rmon_event_tree)) {
        rmon_event_entry_delete(event_ptr);
    }
    return;
}

void
history_fill_data (void *src, void *dst, int no_args, void *arg_struct)
{
}

void
rmon_log_entry_db_delete ()
{
    log_entry_t *log_ptr = NULL;

    if (atavl_is_empty(&rmon_log_tree)) {
        return;
    }

    log_ptr = rmon_get_first_log_entry();
    for (; log_ptr; log_ptr = rmon_get_first_log_entry()) {
        assert(atavl_delete_node(&rmon_log_tree, log_ptr) == ATAVL_OK);
        assert(atavl_delete_node(&rmon_log_event_tree, log_ptr) == ATAVL_OK);
        assert(atavl_delete_node(&rmon_log_time_tree, log_ptr) == ATAVL_OK);
        rmon_free_memory(RMON_LOG_ENTRY, log_ptr);
    }
}

int
rmon_map_hw_to_rmon_ifstat_index (int val)
{
    switch (val) {
    case SYSTEM_STATS_RX_BYTES:
        return RMON_STATS_RX_BYTES;
    case SYSTEM_STATS_RX_UNICAST_FRAMES:
        return RMON_STATS_RX_UNICAST_FRAMES;
    case SYSTEM_STATS_RX_MULTICAST_FRAMES:
        return RMON_STATS_RX_MULTICAST_FRAMES;
    case SYSTEM_STATS_RX_BROADCAST_FRAMES:
        return RMON_STATS_RX_BROADCAST_FRAMES;
    case SYSTEM_STATS_RX_ERROR_FRAMES:
        return RMON_STATS_RX_ERROR_FRAMES;
    case SYSTEM_STATS_RX_CRC_ERROR_FRAMES:
        return RMON_STATS_RX_CRC_ERROR_FRAMES;
    case SYSTEM_STATS_RX_RUNT_FRAMES:
        return RMON_STATS_RX_RUNT_FRAMES;
    case SYSTEM_STATS_RX_DROP_EVENT:
        return RMON_STATS_RX_DROP_EVENT;
    case SYSTEM_STATS_RX_GOOD_OVERSIZED_FRAMES:
        return RMON_STATS_RX_GOOD_OVERSIZED_FRAMES;
    case SYSTEM_STATS_RX_FRAGMENT_FRAMES:
        return RMON_STATS_RX_FRAGMENT_FRAMES;
    case SYSTEM_STATS_RX_JABBER_FRAMES:
        return RMON_STATS_RX_JABBER_FRAMES;
    case SYSTEM_STATS_TX_COLLISION_FRAMES:
        return RMON_STATS_TX_COLLISION_FRAMES;
    default:
        break;
    }
    return -1;
}

int
rmon_map_rmon_to_hw_ifstat_index (int val)
{
    switch (val) {
    case RMON_STATS_RX_BYTES:
        return SYSTEM_STATS_RX_BYTES;
    case RMON_STATS_RX_UNICAST_FRAMES:
        return SYSTEM_STATS_RX_UNICAST_FRAMES;
    case RMON_STATS_RX_MULTICAST_FRAMES:
        return SYSTEM_STATS_RX_MULTICAST_FRAMES;
    case RMON_STATS_RX_BROADCAST_FRAMES:
        return SYSTEM_STATS_RX_BROADCAST_FRAMES;
    case RMON_STATS_RX_ERROR_FRAMES:
        return SYSTEM_STATS_RX_ERROR_FRAMES;
    case RMON_STATS_RX_CRC_ERROR_FRAMES:
        return SYSTEM_STATS_RX_CRC_ERROR_FRAMES;
    case RMON_STATS_RX_RUNT_FRAMES:
        return SYSTEM_STATS_RX_RUNT_FRAMES;
    case RMON_STATS_RX_DROP_EVENT:
        return SYSTEM_STATS_RX_DROP_EVENT;
    case RMON_STATS_RX_GOOD_OVERSIZED_FRAMES:
        return SYSTEM_STATS_RX_GOOD_OVERSIZED_FRAMES;
    case RMON_STATS_RX_FRAGMENT_FRAMES:
        return SYSTEM_STATS_RX_FRAGMENT_FRAMES;
    case RMON_STATS_RX_JABBER_FRAMES:
        return SYSTEM_STATS_RX_JABBER_FRAMES;
    case RMON_STATS_TX_COLLISION_FRAMES:
        return SYSTEM_STATS_TX_COLLISION_FRAMES;
    default:
        break;
    }
    return -1;
}

void
rmon_map_update_history_ifstat_data (uint64_t dst_if_stats[],
                                     uint64_t actual_if_stats, int i)
{
    int map = -1;
    map = rmon_map_hw_to_rmon_ifstat_index(i);

    if (-1 == map) {
        return;
    }
    dst_if_stats[map] = actual_if_stats;
    return;
}

void
rmon_modify_map_ifstat (uint64_t dst_if_stats[],
                        uint64_t src_if_stats[])
{
    int i = 0;
    int map = -1;

    for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
        map = rmon_map_hw_to_rmon_ifstat_index(i);
        if (-1 == map) {
            continue;
        }
        dst_if_stats[map] = src_if_stats[i];
    }
}

void
rmon_update_etherstat_data (uint64_t upd_etherstat_data[],
                            int stat_index)
{
    int                           i = 0;
    int                           j = 0;
    uint64_t
        src_etherstat_data[SYSTEM_STATS_NUM_OF_INTF_ENTRIES];
    etherstat_entry_t            *ptr = NULL;
    rmon_ifstat_data_t           *ifstat_ptr = NULL;
    rmon_etherstat_start_entry_t *etherstat_ptr = NULL;

    for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
        src_etherstat_data[i] = 0;
    }
    etherstat_ptr = rmon_get_etherstat_init_entry(stat_index);
    assert(etherstat_ptr);

    ptr = rmon_get_etherstat_entry(stat_index);
    assert(ptr);

    ifstat_ptr = rmon_get_ifstat_data_entry(ptr->if_index);
    assert(ifstat_ptr);

    for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
        src_etherstat_data[i] = ifstat_ptr->if_stats[i];
    }
    for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
        if (src_etherstat_data[i] < etherstat_ptr->if_stats[i]) {
            upd_etherstat_data[i] = 0;
        } else {
            upd_etherstat_data[i] =
                src_etherstat_data[i] - etherstat_ptr->if_stats[i];
        }
    }
}

void
rmon_get_obj_name_by_oid (char *variable, char *text, int *if_index)
{
    int err_code = -1;
    get_obj_name_by_oid(&g_MIB_OID_struct, variable, text, if_index,
                              &err_code);
}

void
rmon_db_init()
{
    rmon_history_init_tree();
    rmon_init_dirty_tree();
    rmon_ifstat_init_tree();
    rmon_lif_init_tree();
    rmon_alarm_init_tree();
    rmon_event_init_tree();
    rmon_log_init_tree();
    rmon_log_time_tree_init();
    rmon_log_event_tree_init();
    rmon_etherstat_init_tree();
    rmon_etherstat_start_init_tree();
    rmon_ifstat_start_init_tree();
    rmon_mib_oid_init();
}

event_entry_t *
rmon_set_event_entry (event_entry_t *event)
{
    event_entry_t *event_ptr = NULL;

    if (NULL == event) {
        return NULL;
    }

    if (event->event_index < RMON_MIN_EVENT_INDEX) {
        return NULL;
    }
    event_ptr = rmon_get_event_entry(event->event_index);
    if (NULL == event_ptr) {
        event_ptr = rmon_alloc_memory(RMON_EVENT_ENTRY);
        assert(event_ptr);
        memcpy(event_ptr, event, sizeof(event_entry_t));
        assert(atavl_insert(&rmon_event_tree, event_ptr) == ATAVL_OK);
    } else {
        snprintf(event_ptr->community, sizeof(event_ptr->community), "%s",
                 event->community);
        snprintf(event_ptr->owner_str, sizeof(event_ptr->owner_str), "%s",
                 event->owner_str);
        snprintf(event_ptr->desc, sizeof(event_ptr->desc), "%s", event->desc);
        event_ptr->type = event->type;
    }
    return event_ptr;
}

void 
rmon_update_etherstat_entry (etherstat_entry_t *dest_stat_ptr, 
                            etherstat_entry_t *src_stat_ptr)
{
    dest_stat_ptr->stat_index = src_stat_ptr->stat_index;
    dest_stat_ptr->if_index = src_stat_ptr->if_index;
    snprintf(dest_stat_ptr->owner_str, sizeof(dest_stat_ptr->owner_str), "%s",
             src_stat_ptr->owner_str);
    snprintf(dest_stat_ptr->if_name, sizeof(dest_stat_ptr->if_name), "%s",
             src_stat_ptr->if_name);
}

int 
rmon_update_ifstat_data_entry (rmon_ifstat_data_t *data, int if_index)
{
    int                 i;
    int                 j;
    rmon_ifstat_data_t *ifstat_ptr = NULL;

    if (NULL == data) {
        return 0;
    }
    memset(data, 0, sizeof(rmon_ifstat_data_t));

    ifstat_ptr = rmon_get_ifstat_data_entry(if_index);
    if (NULL == ifstat_ptr) {
        return 0;
    }

    memcpy(data->if_stats, ifstat_ptr->if_stats, sizeof(data->if_stats));
    return 1;
}

int 
rmon_set_etherstat_entry (etherstat_entry_t *ptr)
{
    etherstat_entry_t            *etherstat_ptr = NULL;
    etherstat_entry_t             etherstat_data;
    rmon_ifstat_data_t           *ifstat_data_ptr = NULL;
    rmon_ifstat_data_t            ifstat_data;
    rmon_etherstat_start_entry_t *ether_start_ptr = NULL;

    if (!rmon_update_ifstat_data_entry(&ifstat_data, ptr->if_index)) {
        return RMON_FAILURE;
    }

    etherstat_ptr = rmon_get_etherstat_entry(ptr->stat_index);
    if (etherstat_ptr) {
        rmon_update_etherstat_entry(etherstat_ptr, ptr);          
        return RMON_SUCCESS;
    }
    etherstat_ptr = rmon_alloc_memory(RMON_ETHERSTAT_ENTRY);
    assert(etherstat_ptr);

    rmon_update_etherstat_entry(etherstat_ptr, ptr);
    assert(atavl_insert(&rmon_etherstat_tree, etherstat_ptr) == ATAVL_OK);

    /* Now create entry in rmon_etherstat_start_tree */
    ether_start_ptr = rmon_alloc_memory(RMON_ETHERSTAT_START_ENTRY);
    assert(ether_start_ptr);
    ether_start_ptr->stat_index = ptr->stat_index;
    memcpy(ether_start_ptr->if_stats, ifstat_data.if_stats,
           sizeof(ether_start_ptr->if_stats));
    getRtcTime(ether_start_ptr->start_date);
    ether_start_ptr->if_index = ptr->if_index;
    assert(atavl_insert(&rmon_etherstat_start_tree, ether_start_ptr) ==
           ATAVL_OK);

    return RMON_SUCCESS;
}

void
rmon_update_start_etherstat_entry_by_ifindex (uint32_t if_index,
                                              uint64_t if_stats[])
{
    /* Find rmon_etherstat_start_tree if entry matching with this if_index is
     * there or not 
     */
    int                           i;
    rmon_etherstat_start_entry_t *eth_entry = NULL;

    eth_entry = atavl_first(&rmon_etherstat_start_tree);

    while (eth_entry) {
        if (if_index == eth_entry->if_index) {
            for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
                eth_entry->if_stats[i] = if_stats[i];
            }
        }
        eth_entry = atavl_next(&rmon_etherstat_start_tree, eth_entry,
                               ATAVL_NO_DUP);
    }
    return;
}

void
rmon_insert_ifstat_init_entry (int hist_index, rmon_ifstat_data_t
                               *ifstat_data_ptr)
{
    rmon_ifstat_start_entry_t *start_if = NULL;
    start_if = rmon_alloc_memory(RMON_IFSTAT_START_ENTRY);
    assert(start_if);
    start_if->hist_index = hist_index;
    memcpy(start_if->if_stats, ifstat_data_ptr->if_stats,
           sizeof(start_if->if_stats));
    start_if->flag = RMON_IFSTAT_CLEAR_BIT_RESET;
    assert(atavl_insert(&rmon_ifstat_start_tree, start_if) == ATAVL_OK);
}

void
rmon_etherstat_entry_db_delete ()
{
    etherstat_entry_t            *stat_obj = NULL;
    rmon_etherstat_start_entry_t *start_obj = NULL;

    stat_obj = atavl_first(&rmon_etherstat_tree);
    for (; stat_obj; stat_obj = atavl_first(&rmon_etherstat_tree)) {
        start_obj = rmon_get_etherstat_init_entry(stat_obj->stat_index);
        assert(start_obj);
        assert(atavl_delete(&rmon_etherstat_start_tree, start_obj) == ATAVL_OK);
        rmon_free_memory(RMON_ETHERSTAT_START_ENTRY, start_obj);

        assert(atavl_delete(&rmon_etherstat_tree, stat_obj) == ATAVL_OK);
        rmon_free_memory(RMON_ETHERSTAT_ENTRY, stat_obj);
    }
    assert(0 == atavl_count(&rmon_etherstat_start_tree));
    return;
}

void
rmon_delete_etherstat_entry_by_ifindex (int if_index)
{
    etherstat_entry_t            *stat_obj = NULL;
    etherstat_entry_t            *temp_obj = NULL;
    rmon_etherstat_start_entry_t *start_obj = NULL;

    stat_obj = atavl_first(&rmon_etherstat_tree);
    while (stat_obj) {
        if (stat_obj->if_index != if_index) {
            stat_obj = atavl_next(&rmon_etherstat_tree, stat_obj, ATAVL_NO_DUP);
            continue;
        }
        temp_obj = stat_obj;
        stat_obj = atavl_next(&rmon_etherstat_tree, stat_obj, ATAVL_NO_DUP);
        start_obj = rmon_get_etherstat_init_entry(temp_obj->stat_index);
        assert(start_obj);
        assert(atavl_delete(&rmon_etherstat_start_tree, start_obj) == ATAVL_OK);
        rmon_free_memory(RMON_ETHERSTAT_START_ENTRY, start_obj);

#ifndef LINUX_APP
        SYSTEM_MSG_RMON_ETHERSTAT_ENTRY_DELETED(temp_obj->stat_index);
#endif
        assert(atavl_delete(&rmon_etherstat_tree, temp_obj) == ATAVL_OK);
        rmon_free_memory(RMON_ETHERSTAT_ENTRY, temp_obj);
    }
    return;
}

