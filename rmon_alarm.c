/*
 * rmon_alarm.c: Remote MONitoring alarm
 */


#include <atavl/atavl.h>
#include "rmon_defs.h"
#include "rmon_cb.h"

int
rmon_get_last_log_index_by_event_index (int event_index)
{
    int log_index = 0;
    int last_log_index = 0;
    log_entry_t   log_data;
    log_entry_t  *log_ptr = NULL;

    log_ptr = rmon_get_next_log_entry(log_index, event_index);
    if (NULL == log_ptr) {
        return 0;
    }
    while (log_ptr) {
        if (log_ptr->event_index != event_index) {
            return last_log_index;
        }
        last_log_index = log_ptr->log_index;
        log_ptr = (log_entry_t *)atavl_next(&rmon_log_tree, log_ptr,
                                            ATAVL_NO_DUP);
    }
    /* we should not come here */
    return last_log_index;
}

int
rmon_trigger_event (alarm_entry_t *alarm_ptr, int type, int if_index)
{
    int             last_log_index = 0;
    int             log_index = 0;
    int             event_index;
    char            log_desc[RMON_LOG_DESC_LEN];
    log_entry_t     log_data;
    log_entry_t    *log_ptr = NULL;
    event_entry_t  *event_ptr = NULL;

    if (ALARM_RISING == type) {
        event_index = alarm_ptr->rising_event_index;
    } else {
        event_index = alarm_ptr->falling_event_index;
    }
    event_ptr = rmon_get_event_entry(event_index);
    if ((NULL == event_ptr) || (EVENT_TYPE_NONE == event_ptr->type)) {
        return 0;
    }

    memset(log_desc, 0, sizeof(log_desc));

    event_ptr->last_time_sent = rmon_get_sys_uptime();

    if ((EVENT_TYPE_LOG == event_ptr->type) ||
        (EVENT_TYPE_LOG_TRAP == event_ptr->type)) {
        /* Create entry in log table */
        if (ALARM_RISING == type) {
            snprintf(log_desc, RMON_LOG_DESC_LEN, "%s %s",
                     RMON_RISING_THRESHOLD_LOG_STR, alarm_ptr->variable);
        } else {
            snprintf(log_desc, RMON_LOG_DESC_LEN, "%s %s",
                     RMON_FALLING_THRESHOLD_LOG_STR, alarm_ptr->variable);
        }

        log_index =
            rmon_get_last_log_index_by_event_index(event_index) + 1;
        if (RMON_MAX_LOG_INDEX == log_index) {
            /* reached max */
            return 0;
        }
        log_ptr = rmon_alloc_memory(RMON_LOG_ENTRY);
        assert(log_ptr);
        log_ptr->event_index = event_ptr->event_index;
        log_ptr->log_index = log_index;
        strncpy(log_ptr->log_desc, log_desc, sizeof(log_ptr->log_desc));
        log_ptr->log_time = event_ptr->last_time_sent;
        log_ptr->alarm_index = alarm_ptr->alarm_index;
        log_ptr->alarm_value = (int)alarm_ptr->last_seen_value;
        log_ptr->alarm_sample_type = alarm_ptr->sample_type;
        if (ALARM_RISING == type) {
            log_ptr->threshold_value = alarm_ptr->rising_threshold_value;
        } else {
            log_ptr->threshold_value = alarm_ptr->falling_threshold_value;
        }
        assert(atavl_insert(&rmon_log_tree, log_ptr) == ATAVL_OK);
        assert(atavl_insert(&rmon_log_time_tree, log_ptr) == ATAVL_OK);
        assert(atavl_insert(&rmon_log_event_tree, log_ptr) == ATAVL_OK);
    }
    if ((EVENT_TYPE_TRAP == event_ptr->type) ||
        (EVENT_TYPE_LOG_TRAP == event_ptr->type)) {
        if (ALARM_RISING == type) {
            /* Send risingAlarm trap */
        } else {
            /* Send fallingAlarm trap */
        }
    }
    return 1;
}

void
rmon_check_send_trap_log (uint64_t current_val, alarm_entry_t *alarm_ptr)
{
    int      if_index;

    if_index = alarm_ptr->if_index;

    if (RMON_ALARM_INIT_VALUE == alarm_ptr->last_seen_value) {
        /* This is the start of sample */
        alarm_ptr->last_absolute_value = current_val;
        alarm_ptr->last_seen_value = 0;
        return;
    }

    if (RMON_ALARM_SAMPLE_TYPE_DELTA == alarm_ptr->sample_type) {
        alarm_ptr->last_seen_value = current_val -
            alarm_ptr->last_absolute_value;
    } else {
        alarm_ptr->last_seen_value = current_val;
    }

    alarm_ptr->last_absolute_value = current_val;

    if ((alarm_ptr->startup_alarm == ALARM_RISING) ||
        (alarm_ptr->startup_alarm == ALARM_RISING_OR_FALLING)) {
        if ((alarm_ptr->last_seen_value >= alarm_ptr->rising_threshold_value) &&
            (alarm_ptr->last_alarm_seen != ALARM_RISING)) {
            if (rmon_trigger_event(alarm_ptr, ALARM_RISING, if_index)) {
                alarm_ptr->last_alarm_seen = ALARM_RISING;
                /* Next time onwards allow all the events */
                alarm_ptr->startup_alarm = ALARM_RISING_OR_FALLING;
            }
        }
    }
    if ((alarm_ptr->startup_alarm == ALARM_FALLING) ||
               (alarm_ptr->startup_alarm == ALARM_RISING_OR_FALLING)) {
        if ((alarm_ptr->last_seen_value <= alarm_ptr->falling_threshold_value) &&
            (alarm_ptr->last_alarm_seen != ALARM_FALLING)) {
            if (rmon_trigger_event(alarm_ptr, ALARM_FALLING, if_index)) {
                alarm_ptr->startup_alarm = ALARM_RISING_OR_FALLING;
                alarm_ptr->last_alarm_seen = ALARM_FALLING;
            }
        }
    }
}

int
rmon_set_alarm_entry (alarm_entry_t *alarm)
{
    alarm_entry_t     *alarm_ptr = NULL;

    if (NULL == alarm) {
        return RMON_FAILURE;
    }

    /* Check if the entry exists or not */
    alarm_ptr = rmon_get_alarm_entry(alarm->alarm_index);
    if (alarm_ptr) {
        /* Just modify the data */
        memcpy(alarm_ptr, alarm, sizeof(alarm_entry_t));
        return RMON_SUCCESS;
    }

    alarm_ptr = rmon_alloc_memory(RMON_ALARM_ENTRY);
    assert(alarm_ptr);
    memcpy(alarm_ptr, alarm, sizeof(alarm_entry_t));

    assert(atavl_insert(&rmon_alarm_tree, alarm_ptr) == ATAVL_OK);
    rmon_start_alarm_update_timer();
    return RMON_SUCCESS;
}

int
rmon_reset_alarm_entry (alarm_entry_t *alarm_ptr)
{
    assert(atavl_delete(&rmon_alarm_tree, alarm_ptr) == ATAVL_OK);
    rmon_free_memory(RMON_ALARM_ENTRY, alarm_ptr);
    return 1;
}

int 
rmon_reset_alarm_entry_by_index (int alarm_index)
{
    alarm_entry_t *alarm_ptr = NULL;

    alarm_ptr = rmon_get_alarm_entry(alarm_index);
    if (NULL == alarm_ptr) {
        return 0;
    }

    return rmon_reset_alarm_entry(alarm_ptr);
}

void
rmon_alarm_entry_db_delete ()
{
    alarm_entry_t *alarm_ptr = NULL;

    alarm_ptr = atavl_first(&rmon_alarm_tree);
    for (; alarm_ptr; alarm_ptr = atavl_first(&rmon_alarm_tree)) {
        rmon_reset_alarm_entry(alarm_ptr);
    }
    return;
}

void
rmon_delete_alarm_entry_by_ifindex (int if_index)
{
    alarm_entry_t *alarm_ptr = NULL;
    alarm_entry_t *alarm_temp_ptr = NULL;

    alarm_ptr = atavl_first(&rmon_alarm_tree);

    while (alarm_ptr) {
        if (if_index != alarm_ptr->if_index) {
            alarm_ptr = atavl_next(&rmon_alarm_tree, alarm_ptr, ATAVL_NO_DUP);
            continue;
        }
        alarm_temp_ptr = alarm_ptr;
        alarm_ptr = atavl_next(&rmon_alarm_tree, alarm_ptr, ATAVL_NO_DUP);
        rmon_reset_alarm_entry(alarm_temp_ptr);
    }
    return;
}

void 
rmon_alarm_log_event_update (alarm_entry_t *alarm_ptr)
{
    int                  i = 0;
    int                  sample_type = 0;
    int                 *counter_ptr = NULL;
    char                *ptr = NULL;
    char                 var_buff[RMON_OBJ_NAME_LEN];
    uint64_t             current_val = 0;
    rmon_ifstat_data_t  *ifstat_ptr = NULL;
    rmon_ifstat_data_t   ifstat_data;
    alarm_ifstat_counter_entries_t  *ifstat_cntr = NULL;

    sample_type = alarm_ptr->sample_type;
    alarm_ptr->time_left = alarm_ptr->time_left - RMON_ALARAM_UPDATE_TIMER;

    if ((alarm_ptr->time_left <= 0) || 
        (RMON_ALARM_INIT_VALUE == alarm_ptr->last_seen_value)) {
        alarm_ptr->time_left = alarm_ptr->interval;
        /* Need to check now if we need to generate event/log etc */
        snprintf(var_buff, RMON_OBJ_NAME_LEN, "%s", alarm_ptr->variable);
        ptr = strchr(var_buff, '.');
        if (NULL == ptr) {
            return;
        }
        var_buff[strlen(var_buff) - strlen(ptr)] = '\0';
        ifstat_cntr = rmon_find_counter_entry(var_buff);
        if (NULL == ifstat_cntr) {
            return;
        }
        if (!rmon_update_ifstat_data_entry(&ifstat_data,
                                           alarm_ptr->if_index)) {
            return;
        }
        for (i = 0; i < ifstat_cntr->num_counters; i++) { 
            current_val += ifstat_data.if_stats[ifstat_cntr->counters[i]]; 
        }

        rmon_check_send_trap_log(current_val, alarm_ptr);
    }
}

void 
rmon_alarm_update (void *timer, void *arg)
{
    alarm_entry_t  *alarm_ptr = NULL;
    alarm_ptr = (alarm_entry_t *)atavl_first(&rmon_alarm_tree);
    if (NULL == alarm_ptr) {
        return;
    }
    while (alarm_ptr) {
        rmon_alarm_log_event_update(alarm_ptr);
        alarm_ptr = (alarm_entry_t *)atavl_next(&rmon_alarm_tree, alarm_ptr,
                                                ATAVL_NO_DUP);
    }
    return;
}

void
rmon_start_alarm_update_timer ()
{
    rmon_timer_create(RMON_TIMER_TYPE_ALARM, RMON_ALARAM_UPDATE_TIMER,
                      rmon_alarm_update, NULL);
}

