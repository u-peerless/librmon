/*
 * rmon_history.c
 */

#include "atavl.h"
#include "rmon_defs.h"
#include "rmon_cb.h"

history_table_t *
rmon_insert_dirty_entry (int hist_index)
{
    history_table_t       *hist_ptr = NULL;
    rmon_hist_ctrl_data_t *hist_ctrl_ptr = NULL;

    hist_ptr = rmon_alloc_memory(RMON_HISTORY_DIRTY_ENTRY);
    assert(hist_ptr);
    hist_ptr->history_index = hist_index;
    hist_ctrl_ptr = rmon_alloc_memory(RMON_HISTORY_CTRL_ENTRY);
    assert(hist_ctrl_ptr);
    hist_ptr->data = hist_ctrl_ptr;
    assert(atavl_insert(&history_dirty_entry_tree, hist_ptr) == ATAVL_OK);
    return hist_ptr;
}

void
rmon_delete_dirty_entry (int hist_index)
{
    history_table_t       *hist_ptr = NULL;
    rmon_hist_ctrl_data_t *hist_ctrl_ptr = NULL;

    hist_ptr = rmon_history_get_dirty_entry(hist_index);
    if (NULL == hist_ptr) {
        return;
    }
    hist_ctrl_ptr = hist_ptr->data;
    rmon_free_memory(RMON_HISTORY_CTRL_ENTRY, hist_ctrl_ptr);
    assert(atavl_delete(&history_dirty_entry_tree, hist_ptr) == ATAVL_OK);
    rmon_free_memory(RMON_HISTORY_DIRTY_ENTRY, hist_ptr);
    return;
}

void
rmon_update_history_sample_index (history_data_t *hist_data_ptr)
{
    rmon_ifstat_entry_t *curr_ifstat = NULL;
    rmon_ifstat_entry_t *prev_ifstat = NULL;

    curr_ifstat = (rmon_ifstat_entry_t *)hist_data_ptr->data;
    prev_ifstat = (rmon_ifstat_entry_t *)hist_data_ptr->blink->data;
    curr_ifstat->sample_index = prev_ifstat->sample_index + 1;
}

void
rmon_update_history_ifstat_data (uint64_t dst_if_stats[],
                                 uint64_t curr_if_stats[])
{
    int i = 0;
    int map = -1;

    memset(dst_if_stats, 0, RMON_STATS_NUM_OF_INTF_ENTRIES * sizeof(uint64_t));

    for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
        map = rmon_map_hw_to_rmon_ifstat_index(i);
        if (-1 == map) {
            continue;
        }
        dst_if_stats[map] = curr_if_stats[i];
    }
    return;
}

int
rmon_update_if_utilization (uint64_t prev_ifstat[],
                            uint64_t curr_ifstat[],
                            history_table_t *hist_ptr,
                            int is_first_sample)

{
    int                        if_util = 0;
    uint64_t                   start_buff[RMON_STATS_NUM_OF_INTF_ENTRIES];
    uint64_t                   curr_buff[RMON_STATS_NUM_OF_INTF_ENTRIES];
    uint64_t                   start_pkts_count = 0;
    uint64_t                   prev_pkts_count = 0;
    uint64_t                   curr_pkts_count = 0;
    uint64_t                   start_octets_count = 0;
    uint64_t                   prev_octets_count = 0;
    uint64_t                   curr_octets_count = 0;
    rmon_ifstat_start_entry_t *start_if = NULL;

    start_if = rmon_get_ifstat_init_entry(hist_ptr->history_index);
    assert(start_if);

    rmon_modify_map_ifstat(start_buff, start_if->if_stats);
    start_pkts_count = rmon_get_if_pkts_count(start_buff);
    start_octets_count = start_if->if_stats[SYSTEM_STATS_RX_BYTES];

    prev_pkts_count = rmon_get_if_pkts_count(prev_ifstat);
    prev_octets_count = prev_ifstat[RMON_STATS_RX_BYTES];

    rmon_modify_map_ifstat(curr_buff, curr_ifstat);
    curr_pkts_count = rmon_get_if_pkts_count(curr_buff);
    curr_octets_count = curr_ifstat[SYSTEM_STATS_RX_BYTES];

    if ((prev_pkts_count  > curr_pkts_count) ||
         (prev_octets_count > curr_octets_count) || 
        /* Happens when all the samples got created, and this sample was the
         * oldest sample so far
         */
        (is_first_sample)) {
        prev_pkts_count = start_pkts_count;
        prev_octets_count = start_octets_count;
    }

    if (RMON_IFSTAT_CLEAR_BIT_SET == start_if->flag) {
        /* 1st sample after clear counters been done */
        prev_pkts_count = 0;
        prev_octets_count = 0;
        start_if->flag = RMON_IFSTAT_FIRST_PASS;
    }

    if ((prev_pkts_count > curr_pkts_count) ||
        (prev_octets_count  > curr_octets_count)) {
        /* If Still curr pkt count is less, then just return 0 */
        return 0;
    }
    if_util = (((curr_pkts_count - prev_pkts_count)
                 * (9.6 + 6.4)) +
        ((curr_octets_count - prev_octets_count)
          * 0.8)) /
        (hist_ptr->interval * 10000);
    return if_util;
}

void
rmon_insert_history_data_entry_queue (history_table_t *hist_tbl,
                                      history_data_t *hist_data)
{
    if (NULL == hist_tbl->head_t) {
        hist_tbl->head_t = hist_data;
        hist_tbl->last_t = hist_data;
        hist_data->flink = hist_data;
        hist_data->blink = hist_data;
    } else {
        hist_data->flink = hist_tbl->head_t;
        hist_tbl->head_t->blink = hist_data;
        hist_tbl->last_t->flink = hist_data;
        hist_data->blink = hist_tbl->last_t;
        hist_tbl->last_t = hist_data;
    } 
    return;
}

void
rmon_history_entry_delete (history_table_t *hist_table)
{
    history_data_t            *hist_data_ptr = NULL;
    history_data_t            *temp_hist_data_ptr = NULL;
    history_table_t           *hist_dirt_obj = NULL;
    rmon_ifstat_entry_t       *rmon_ifstat = NULL;
    rmon_hist_ctrl_data_t     *hist_ctrl_data = NULL;
    rmon_ifstat_start_entry_t *ifstat_start_ptr = NULL;

    hist_dirt_obj = rmon_history_get_dirty_entry(hist_table->history_index);
    if (hist_dirt_obj) {
        rmon_delete_dirty_entry(hist_dirt_obj->history_index);
    }

    hist_ctrl_data = (rmon_hist_ctrl_data_t *)hist_table->data;
    if (hist_ctrl_data) {
        rmon_free_memory(RMON_HISTORY_CTRL_ENTRY, hist_ctrl_data);
    }
    /* Now delete all the samples created */
    hist_data_ptr = hist_table->head_t;
    while ((hist_data_ptr) &&(hist_data_ptr != hist_table->last_t)) {
        rmon_ifstat = (rmon_ifstat_entry_t *)hist_data_ptr->data;
        if (rmon_ifstat) {
            rmon_free_memory(RMON_IFSTAT_ENTRY, rmon_ifstat);
        }
        temp_hist_data_ptr = hist_data_ptr;
        hist_data_ptr = hist_data_ptr->flink;
        rmon_free_memory(RMON_HISTORY_DATA_ENTRY, temp_hist_data_ptr);
    }
    if (hist_data_ptr != NULL) {
    /* now delete the last entries */
        rmon_ifstat = (rmon_ifstat_entry_t *)hist_data_ptr->data;
        if (rmon_ifstat) {
            rmon_free_memory(RMON_IFSTAT_ENTRY, rmon_ifstat);
        }
        rmon_free_memory(RMON_HISTORY_DATA_ENTRY, hist_data_ptr);
    }
    /* Delete the entry from rmon_ifstat_start_tree */
    ifstat_start_ptr = rmon_get_ifstat_init_entry(hist_table->history_index);
    if (ifstat_start_ptr != NULL) {
        /* It may happen that user has changed the interval or monitor-entity
         * before first sample created
         */
        assert(atavl_delete(&rmon_ifstat_start_tree, ifstat_start_ptr) == ATAVL_OK);
        rmon_free_memory(RMON_IFSTAT_START_ENTRY, ifstat_start_ptr);
    }
    /* Now delete this history entry */
    assert(atavl_delete(&history_table_tree, hist_table) == ATAVL_OK);
    rmon_free_memory(RMON_HISTORY_ENTRY, hist_table);
    return;
}

void
rmon_history_entry_db_delete ()
{
    history_table_t           *hist_obj = NULL;

    hist_obj = atavl_first(&history_table_tree);
    for (; hist_obj; hist_obj = atavl_first(&history_table_tree)) {
        rmon_history_entry_delete(hist_obj);
    }
    return;
}

void
rmon_delete_history_entry_by_ifindex (int if_index)
{
    history_table_t         *hist_obj = NULL;
    history_table_t         *hist_temp_obj = NULL;
    rmon_hist_ctrl_data_t   *hist_ctrl_data = NULL;

    hist_obj = atavl_first(&history_table_tree);
    while (hist_obj) {
        hist_ctrl_data = hist_obj->data;
        if (NULL == hist_ctrl_data) {
            hist_obj = atavl_next(&history_table_tree, hist_obj, ATAVL_NO_DUP);
            continue;
        }
        if (if_index != hist_ctrl_data->if_index) {
            hist_obj = atavl_next(&history_table_tree, hist_obj, ATAVL_NO_DUP);
            continue;
        }
        hist_temp_obj = hist_obj;
        hist_obj = atavl_next(&history_table_tree, hist_obj, ATAVL_NO_DUP);
        rmon_history_entry_delete(hist_temp_obj);
    }
}

void
rmon_create_history_buckets (history_table_t *hist_table, int requested)
{
    int                     i = 0;
    history_data_t         *hist_data_ptr = NULL;
    rmon_ifstat_entry_t    *rmon_ifstat = NULL;

    while (i < requested) {
        hist_data_ptr = rmon_alloc_memory(RMON_HISTORY_DATA_ENTRY);
        rmon_ifstat = rmon_alloc_memory(RMON_IFSTAT_ENTRY);
        if ((NULL == hist_data_ptr) || (NULL == rmon_ifstat)) {
            hist_table->granted = i;
            return;
        }
        hist_data_ptr->data = rmon_ifstat;
        rmon_insert_history_data_entry_queue(hist_table, hist_data_ptr);
        i++;
    }
    hist_table->granted = i;
}

history_table_t *
history_register_table (int requested, int interval,
                        int size, int history_index,
                       // history_callback_fn_t fn,
                        atavl_tree_t *tree)
{
    int              i = 0;
    history_table_t *hist_ptr = NULL;
    history_table_t  hist_data;

    if (!interval) {
        return NULL;
    }

    hist_data.history_index = history_index;

    hist_ptr = (history_table_t *)atavl_find(tree, &hist_data);
    if (hist_ptr != NULL) {
        /* Entry already exists */
        return NULL;
    }
    hist_ptr = rmon_alloc_memory(RMON_HISTORY_ENTRY);
    assert(hist_ptr);

    hist_ptr->requested = requested;
    /* Let us assign the maximum granted, if fails we will see while inserting
       into queue */
    hist_ptr->granted = requested;
    hist_ptr->interval = interval;
    hist_ptr->samples_created_count = 0;
    hist_ptr->history_index = history_index;
    hist_ptr->entry_status = RMON_ENTRY_VALID;
    hist_ptr->head_t = NULL;
    hist_ptr->last_t = NULL;
    hist_ptr->update_t = NULL;
    assert(atavl_insert(tree, hist_ptr) == ATAVL_OK);
    return hist_ptr;
}

history_table_t *
rmon_history_entry_modify_by_dirty_entry (history_table_t *hist_table,
                                          history_table_t *dirty_entry)
{
    int                     i = 0;
    int                     requested = 0;
    int                     num_delete = 0;
    int                     num_add = 0;
    int                     history_index;
    int                     interval;
    int                     if_index;
    char                    owner_string[RMON_OWNER_STR_SIZE];
    history_data_t         *hist_data_ptr = NULL;
    history_data_t         *hist_data_temp_ptr = NULL;
    history_data_t         *hist_data_prev_ptr = NULL;
    rmon_ifstat_entry_t    *rmon_ifstat = NULL;
    rmon_hist_ctrl_data_t  *hist_ctrl_data = NULL;
    rmon_hist_ctrl_data_t  *hist_ctrl_dirty_data = NULL;

    /* Do not delete the entries from Circular List now, insert this updation in
     * dirty tree, and then when history timer expires, then update this data
     */

    requested = dirty_entry->requested;

    hist_table->requested = requested;
    hist_ctrl_data       = (rmon_hist_ctrl_data_t *)hist_table->data;
    hist_ctrl_dirty_data = (rmon_hist_ctrl_data_t *)dirty_entry->data;

    /* Check which parameter has changed */
    if ((hist_table->interval != dirty_entry->interval) ||
        (hist_ctrl_data->if_index != hist_ctrl_dirty_data->if_index)) {
        /* Change of interval is not allowed, so delete the older history and
         * create a new one freshly 
         */
        snprintf(owner_string, sizeof(owner_string), "%s",
                 hist_ctrl_data->owner_string);
        rmon_history_entry_delete(hist_table);

        interval = dirty_entry->interval;
        history_index = dirty_entry->history_index;
        if_index = hist_ctrl_dirty_data->if_index;

        rmon_delete_dirty_entry(dirty_entry->history_index);
        return  history_register(requested, interval, if_index,
                                 history_index, owner_string);
    }

    if (dirty_entry->requested == hist_table->granted) {
        return hist_table;
    }

    hist_table->requested = dirty_entry->requested;
    rmon_delete_dirty_entry(hist_table->history_index);

    if (requested < hist_table->granted) {
        /* Delete the entries granted - requested */
        /* Check how many samples already updated
           1. If samples crated (samples_cleared_count) is less than requested,
              then delete from pointer pointing to requested th entry to 
              last_t pointer, 
                 Ex: Earlier Granted 20
                     New Requestd    15
                     Samples Created 10
                approach: delete from last_t to last (20-15) 5 nodes
           2. If samples created is more than requested, then we need to delete
              the entries as below:
                 Ex: Earlier Granted 20,
                     New Requestd    10 
                     Samples Created 15
              Then first delete from update_t pointer to last_t (so delete
              20-15 = 5 entries) and then delete head_t to first 15-10 = 5 
              entries
         */
#if 0
        if (hist_table->samples_created_count == hist_table->granted) {
            /* Then delete from the head pointer to granted - requested number
             * of entries
             */
            hist_data_temp_ptr = NULL;
            num_delete = hist_table->granted - requested;
            for (i = 0; i < num_delete; i++) {
                hist_data_temp_ptr = hist_table->head_t->flink;
                rmon_delete_history_data(hist_table->head_t);
                hist_table->head_t = hist_data_temp_ptr;
            }
            /* Now adjust the links */
            hist_table->last_t->flink = hist_table->head_t;
            hist_table->head_t->blink = hist_table->last_t;
            hist_table->update_t = hist_table->last_t;
            hist_table->granted = requested;
            return;
        }
#endif

        if (hist_table->samples_created_count <= requested) {
            rmon_hist_ctrl_data_t *ptr = NULL;
            /* Case 1 */
            hist_data_temp_ptr = NULL;
            num_delete = hist_table->granted - requested;
            for (i = 0; i < num_delete; i++) {
                hist_data_temp_ptr = hist_table->last_t->blink;
                ptr = hist_data_temp_ptr->data;
                rmon_delete_history_data(hist_table->last_t);
                hist_table->last_t = hist_data_temp_ptr;
            }
            /* Now adjust the links */
            hist_table->last_t->flink = hist_table->head_t;
            hist_table->head_t->blink = hist_table->last_t;
            hist_table->granted = requested;
        } else {
            /* Case 2 */
            hist_data_temp_ptr = NULL;
            hist_data_temp_ptr = hist_table->last_t;
            while (hist_data_temp_ptr != hist_table->update_t) {
                hist_data_temp_ptr = hist_table->last_t->blink;
                rmon_delete_history_data(hist_table->last_t);
                hist_table->last_t = hist_data_temp_ptr;
            }
            /* We are done with deleting last entries, now delete older entries,
             * i.i enries from head_t
             */
            hist_data_temp_ptr = NULL;
            num_delete = hist_table->samples_created_count - requested;
            for (i = 0; i < num_delete; i++) {
                hist_data_temp_ptr = hist_table->head_t->flink;
                rmon_delete_history_data(hist_table->head_t);
                hist_table->head_t = hist_data_temp_ptr;
            }
            /* Now adjust the links */
            hist_table->last_t->flink = hist_table->head_t;
            hist_table->head_t->blink = hist_table->last_t;
            hist_table->update_t = hist_table->last_t;
            hist_table->granted = requested;
            hist_table->samples_created_count = hist_table->granted;
        }
    } else {
        num_add = requested - hist_table->granted;
        for (i = 0; i < num_add; i++) {
            hist_data_ptr = rmon_alloc_memory(RMON_HISTORY_DATA_ENTRY);
            rmon_ifstat   = rmon_alloc_memory(RMON_IFSTAT_ENTRY);
            if ((NULL == hist_data_ptr) || (NULL == rmon_ifstat)) {
                hist_table->granted += i;
                return hist_table;
            }
            hist_data_ptr->data = rmon_ifstat;
            rmon_insert_history_data_entry_queue(hist_table, hist_data_ptr);
        }
        hist_table->granted += i;
    }
    return hist_table;
}

void 
rmon_update_ifstat_start_entry (int hist_index, history_data_t *hist_data_ptr)
{
    int                        i = 0;
    int map = -1;
    rmon_ifstat_entry_t       *ifstat_head_ptr = NULL;
    rmon_ifstat_start_entry_t *ifstat_start_entry = NULL;

    ifstat_start_entry = rmon_get_ifstat_init_entry(hist_index);
    assert(ifstat_start_entry);

    memset(ifstat_start_entry->if_stats, 0,
           sizeof(ifstat_start_entry->if_stats));

    ifstat_head_ptr = (rmon_ifstat_entry_t *)hist_data_ptr->data;
    for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
        map = rmon_map_hw_to_rmon_ifstat_index(i);
        if (-1 == map) {
            continue;
        }
        ifstat_start_entry->if_stats[i] = ifstat_head_ptr->if_stats[map];
    }
    return;
}

void
history_fetch_data (void *data)
{
    int                        i = 0;
    int                        time_left = 0;
    uint64_t                   dummy_prev_ifstat[SYSTEM_STATS_NUM_OF_INTF_ENTRIES];
    history_data_t            *hist_data = NULL;
    history_data_t            *hist_data_ptr = NULL;
    history_data_t            *hist_update_ptr = NULL;
    history_table_t           *hist_table = NULL;
    history_table_t           *hist_dirty_entry = NULL;
    rmon_ifstat_data_t        *ifstat_data_ptr = NULL;
    rmon_ifstat_data_t         ifstat_data;
    rmon_ifstat_entry_t       *ifstat = NULL;
    rmon_ifstat_entry_t       *ifstat_ptr = NULL;
    rmon_hist_ctrl_data_t     *hist_ctrl_ptr = NULL;
    rmon_ifstat_start_entry_t *start_if = NULL;

    hist_table = (history_table_t *)data;
    assert(hist_table);
    hist_ctrl_ptr = hist_table->data;
    if ((NULL == hist_ctrl_ptr) || (!hist_table->granted)) {
        return;
    }

    hist_data_ptr = hist_table->update_t;
    if (hist_data_ptr) {
        ifstat_ptr = (rmon_ifstat_entry_t *)hist_data_ptr->data;
        if (ifstat_ptr) {
            /* If Sample Index reaches Max value, then strict to that value */
            if (RMON_MAX_HISTORY_SAMPLE_INDEX == ifstat_ptr->sample_index) {
                /* We will reach here RMON_MAX_HISTORY_SAMPLE_INDEX * 8 seconds
                 * after the history entry is created at min
                 */
                return;
            }
        }
    }

    ifstat_data_ptr = rmon_get_ifstat_data_entry(hist_ctrl_ptr->if_index);
    if (NULL == ifstat_data_ptr) {
        return;
    }

#if 0
    if (atavl_count(&history_dirty_entry_tree)) {
        hist_dirty_entry = rmon_history_get_dirty_entry(hist_table->history_index);
        if (hist_dirty_entry) {
            /* There is an indication to modify the parameters of thie history entry
             */
            hist_table = rmon_history_entry_modify_by_dirty_entry(hist_table, hist_dirty_entry);
            if (NULL == hist_table) {
                /* History registration failed */
                return;
            }
        }
    }
#endif

    if ((0 == hist_table->samples_created_count) && 
        (hist_table->time_left == hist_table->interval)) {
        rmon_insert_ifstat_init_entry(hist_table->history_index,
                                      ifstat_data_ptr);
    }

    hist_table->time_left -= RMON_HISTORY_UPDATE_TIMER;
    time_left = hist_table->time_left;

    if ((hist_table->samples_created_count < hist_table->granted) &&
        (time_left <= 0)) {
        hist_table->time_left = hist_table->interval;
        if (0 == hist_table->samples_created_count) {
            hist_table->update_t = hist_table->head_t;
            hist_data = hist_table->update_t;
            ifstat = (rmon_ifstat_entry_t *)hist_data->data;
            assert(ifstat);
            /* Calculate Utilization from rmon_ifstat_start_entry_t */
            /* This is the sample first entry going to create */
            start_if = rmon_get_ifstat_init_entry(hist_table->history_index);
            assert(start_if);
            memset(dummy_prev_ifstat, 0, sizeof(dummy_prev_ifstat));
            rmon_update_history_ifstat_data(ifstat->if_stats,
                                            ifstat_data_ptr->if_stats);
            ifstat->if_util = 
                rmon_update_if_utilization(dummy_prev_ifstat,
                                           ifstat_data_ptr->if_stats,
                                           hist_table, 1);
        } else {
            hist_table->update_t = hist_table->update_t->flink;
            hist_data = hist_table->update_t;
            ifstat = (rmon_ifstat_entry_t *)hist_data->data;
            assert(ifstat);

            rmon_update_history_ifstat_data(ifstat->if_stats,
                                            ifstat_data_ptr->if_stats);
           ifstat->if_util =
               rmon_update_if_utilization(
                ((rmon_ifstat_entry_t *)hist_data->blink->data)->if_stats,
                 ifstat_data_ptr->if_stats, hist_table, 0);
        }
        snprintf(hist_ctrl_ptr->if_name, sizeof(hist_ctrl_ptr->if_name), "%s",
                 ifstat_data_ptr->if_name);
        ifstat->timestamp = rmon_get_sys_uptime();
        hist_table->samples_created_count++;
        if (hist_table->samples_created_count == hist_table->granted) {
            /* We have created all the samples, next time onwards just update it
             * */
            assert(hist_table->update_t == hist_table->last_t);
            hist_table->time_left = hist_table->interval;
        }
        rmon_update_history_sample_index(hist_table->update_t);
    } else if (time_left <= 0) {
        /* Update the start node with the start pointer */
        rmon_update_ifstat_start_entry(hist_table->history_index,
                                       hist_table->head_t);
        hist_table->head_t = hist_table->head_t->flink;
        hist_table->last_t = hist_table->last_t->flink;
        hist_table->update_t = hist_table->last_t;
        rmon_update_history_sample_index(hist_table->update_t);
        hist_table->time_left = hist_table->interval;
        hist_data = hist_table->update_t;
        ifstat = (rmon_ifstat_entry_t *)hist_data->data;
        rmon_update_history_ifstat_data(ifstat->if_stats,
                                        ifstat_data_ptr->if_stats);
        ifstat->if_util =
            rmon_update_if_utilization(
            ((rmon_ifstat_entry_t *)hist_data->blink->data)->if_stats,
             ifstat_data_ptr->if_stats, hist_table, 0);
        ifstat->timestamp = rmon_get_sys_uptime();
    }
}

void 
rmon_history_update (void *timer, void *arg)
{
    int              i = 0;
    int              if_index = 0;
    /* Update all the entries which are in history_table_tree */
    history_data_t     *his_data = NULL;
    history_table_t *hist_table = NULL;
    rmon_ifstat_data_t *ifstat_data_ptr = NULL;
    rmon_ifstat_data_t  ifstat_data;
    rmon_hist_ctrl_data_t *ctrl_data = NULL;

    hist_table = (history_table_t *)atavl_first(&history_table_tree);
    if (NULL == hist_table) {
        return;
    }
    
    while (hist_table) {
        history_fetch_data(hist_table);
        hist_table = (history_table_t *)atavl_next(&history_table_tree,
                                                   hist_table, ATAVL_NO_DUP);
    }
    return;
}

void
rmon_start_history_update_timer ()
{
    rmon_timer_create(RMON_TIMER_TYPE_HISTORY, RMON_HISTORY_UPDATE_TIMER,
                      rmon_history_update, NULL);
}

history_table_t *
history_register (int requested, int interval, int if_index, int his_ctrl_id,
                  char *owner_string)
{
    char                      *if_name = NULL;
    history_table_t           *his_table = NULL;
    rmon_hist_ctrl_data_t     *hist_ctrl_data = NULL;

    his_table = history_get_entry(his_ctrl_id);
    if (his_table) {
        return his_table;
    }

    if_name = rmon_get_ifname_by_ifindex(if_index);
    if (NULL == if_name){
        return NULL;
    }

    his_table = history_register_table(requested, interval,
                                       sizeof(history_table_t),
                                       his_ctrl_id, &history_table_tree);
    if (NULL == his_table) {
        return NULL;
    }
    hist_ctrl_data = rmon_alloc_memory(RMON_HISTORY_CTRL_ENTRY);
    assert(hist_ctrl_data);
    his_table->data = hist_ctrl_data;
    his_table->time_left = his_table->interval;
    his_table->requested = requested;
    his_table->samples_created_count = 0;
    memset(hist_ctrl_data, 0, sizeof(rmon_hist_ctrl_data_t));
    hist_ctrl_data->if_index = if_index;
    snprintf(hist_ctrl_data->if_name, sizeof(hist_ctrl_data->if_name), "%s",
             if_name);
    snprintf(hist_ctrl_data->owner_string, sizeof(hist_ctrl_data->owner_string),
             "%s", owner_string);

    rmon_create_history_buckets(his_table, requested);
    return his_table;
}

/* This function is used to get the history sample data
 * If current node is head_t, then subtract from the start node, other wise,
 * subtract from the blink node
 */
void
rmon_update_ifstat_data (uint64_t update_ifstat[], 
                         history_data_t *hist_data_ptr, history_table_t
                         *hist_table_ptr)
{
    int                        i = 0;
    int                        map = -1;
    uint64_t                   buff[SYSTEM_STATS_NUM_OF_INTF_ENTRIES];
    history_data_t            *hist_prev_data_ptr = NULL;
    rmon_ifstat_entry_t       *ifstat_ptr = NULL;
    rmon_ifstat_entry_t       *ifstat_prev_ptr = NULL;
    rmon_ifstat_start_entry_t *ifstat_start_entry = NULL;

    memset(buff, 0, sizeof(buff));

    ifstat_start_entry = rmon_get_ifstat_init_entry(hist_table_ptr->history_index);
    assert(ifstat_start_entry);
    for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
        buff[i] = ifstat_start_entry->if_stats[i];
    }

    ifstat_ptr = hist_data_ptr->data;
    hist_prev_data_ptr = hist_data_ptr->blink;

    for (i = 0; i < SYSTEM_STATS_NUM_OF_INTF_ENTRIES; i++) {
        map = rmon_map_hw_to_rmon_ifstat_index(i);
        if (-1 == map) {
            continue;
        }
        if ((hist_table_ptr->head_t == hist_data_ptr) || 
            (NULL == hist_prev_data_ptr)) {
            update_ifstat[map] = (buff[i] > ifstat_ptr->if_stats[map]) ?
                ifstat_ptr->if_stats[map] :
                (ifstat_ptr->if_stats[map] - buff[i]);
            if ((update_ifstat[map]) || (ifstat_ptr->if_stats[map])) {
                /* Only log non-zero data */
            }
        } else {
            if (hist_prev_data_ptr) {
                ifstat_prev_ptr = hist_prev_data_ptr->data;
                update_ifstat[map] = 
                    (ifstat_prev_ptr->if_stats[map] > ifstat_ptr->if_stats[map])
                    ? ifstat_ptr->if_stats[map] : (ifstat_ptr->if_stats[map] -
                                                   ifstat_prev_ptr->if_stats[map]);
                if ((update_ifstat[map]) || (ifstat_prev_ptr->if_stats[map]) ||
                    (ifstat_ptr->if_stats[map])) {
                    /* Only log non-zero data */
                }
            }
        }
    }
}

void
rmon_update_start_history_entry_by_ifindex (int32_t if_index,
                                            uint64_t if_stats[])
{
    int                        i = 0;
    history_table_t           *hist_ptr = NULL;
    rmon_hist_ctrl_data_t     *hist_ctrl = NULL;
    rmon_ifstat_start_entry_t *ifstat_start = NULL;

    ifstat_start = atavl_first(&rmon_ifstat_start_tree);
    while (ifstat_start) {
        hist_ptr = history_get_entry(ifstat_start->hist_index);
        if (NULL == hist_ptr) {
            ifstat_start = atavl_next(&rmon_ifstat_start_tree, ifstat_start,
                                      ATAVL_NO_DUP);
            continue;
        }
        hist_ctrl = (rmon_hist_ctrl_data_t *)hist_ptr->data;
        if (NULL == hist_ctrl) {
            ifstat_start = atavl_next(&rmon_ifstat_start_tree, ifstat_start,
                                      ATAVL_NO_DUP);
            continue;
        }
        if (if_index == hist_ctrl->if_index) {
            ifstat_start->flag = RMON_IFSTAT_CLEAR_BIT_SET;
        }
        ifstat_start = atavl_next(&rmon_ifstat_start_tree, ifstat_start,
                                  ATAVL_NO_DUP);
    }
    return;
}

