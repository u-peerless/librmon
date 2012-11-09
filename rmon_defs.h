/*
 * rmon_defs.h
 */

#ifndef __RMON_DEFS_H__
#define __RMON_DEFS_H__

/* Define Macros - Start */

#define RMON_IF_NAME_SIZE      32
#define RMON_OWNER_STR_SIZE   128
#define RMON_OBJ_NAME_LEN      64
#define RMON_LOG_DESC_LEN     100
#define RMON_DATE_LEN          40
#define RMON_MAX_IFSTAT_COUNTER_PER_IF  10
#define RMON_ALARM_INIT_VALUE  (-5)
#define RMON_PROFILE_STR_LEN   64

#define RMON_RISING_THRESHOLD_LOG_STR "Rising threshold log:"
#define RMON_FALLING_THRESHOLD_LOG_STR "Falling threshold log:"
#define RMON_MSM_BULK_SYNC_PENDING_IF_NAME " "

#define RMON_MAX_LOG_INDEX  2147483647
#define RMON_MAX_HISTORY_SAMPLE_INDEX 2147483647
#define RMON_MIN_EVENT_INDEX 1

#define RMON_TRACE_FLAG_NONE    0x00000000
#define RMON_TRACE_FLAG_GENERIC 0x00000001
#define RMON_TRACE_FLAG_ALARM   0x00000002
#define RMON_TRACE_FLAG_HISTORY 0x00000004
#define RMON_TRACE_FLAG_LOG     0x00000008
#define RMON_TRACE_FLAG_EVENT   0x00000010
#define RMON_TRACE_FLAG_SNMP    0x00000020
#define RMON_TRACE_FLAG_IFSTAT  0x00000040
#define RMON_TRACE_FLAG_CLI     0x00000080
#define RMON_TRACE_FLAG_MSM     0x00000100
#define RMON_TRACE_FLAG_ALL     0xFFFFFFFF

#define RMON_TRACE_LEVEL_EMERG      0
#define RMON_TRACE_LEVEL_ALERT      1
#define RMON_TRACE_LEVEL_CRIT       2
#define RMON_TRACE_LEVEL_ERR        3
#define RMON_TRACE_LEVEL_WARN       4
#define RMON_TRACE_LEVEL_NOTICE     5
#define RMON_TRACE_LEVEL_INFO       6
#define RMON_TRACE_LEVEL_DEBUG      7

#define RMON_ENTRY_NOT_EXIST        0
#define RMON_ENTRY_SET_NOT_ALLOWED -1 
#define RMON_ENTRY_FAILED          -2
#define RMON_ENTRY_MODIFIABLE      -3
#define RMON_ENTRY_EXISTS          -4

#define RMON_ENTRY_STATE_DELETE_PENDING 1
#define MAX_LAG_MBRS_CNT                8
#define RMON_EVENT_DESC_LEN           128
#define RMON_COMMUNITY_STR_LEN        128
#define RMON_PROF_NAME_MAX              8

#define RMON_ALARAM_UPDATE_TIMER  2
#define RMON_HISTORY_UPDATE_TIMER 2

#define RMON_TIMER_TYPE_ALARM       0
#define RMON_TIMER_TYPE_HISTORY     1
#define RMON_TIMER_TYPE_CONFIG_SYNC 2

#define RMON_IFSTAT_CLEAR_BIT_SET   1
#define RMON_IFSTAT_CLEAR_BIT_RESET 2
#define RMON_IFSTAT_FIRST_PASS      3

/* RMON Error Code */
#define RMON_SUCCESS                0
#define RMON_FAILURE                1
#define RMON_DATA_NOT_EXIST    2

/* RMON tracing macro definitions */
#define RMON_TRACE_INTERNAL(level, flag, str, fmt, arg...) {\
    do_rmon_trace(level, flag, str " " fmt, ##arg);\
}

#define RMON_TRACE_GENERIC(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_GENERIC, \
                        "[RMON_GENERIC]", fmt, ##arg)

#define RMON_TRACE_ALARM(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_ALARM, \
                        "[RMON_ALARM]", fmt, ##arg)

#define RMON_TRACE_LOG(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_LOG, \
                        "[RMON_LOG]", fmt, ##arg)

#define RMON_TRACE_HISTORY(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_HISTORY, \
                        "[RMON_HISTORY]", fmt, ##arg)

#define RMON_TRACE_EVENT(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_EVENT, \
                        "[RMON_EVENT]", fmt, ##arg)

#define RMON_TRACE_IFSTAT(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_IFSTAT, \
                        "[RMON_IFSTAT]", fmt, ##arg)

#define RMON_TRACE_MSM(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_MSM, \
                        "[RMON_MSM]", fmt, ##arg)

#define RMON_TRACE_SNMP(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_SNMP, \
                        "[RMON_SNMP]", fmt, ##arg)

#define RMON_TRACE_CLI(level, fmt, arg...) \
    RMON_TRACE_INTERNAL(level, RMON_TRACE_FLAG_CLI, \
                        "[RMON_CLI]", fmt, ##arg)

/* Define Macros - End */

/* Define Enums - Start */
typedef enum alarm_startup_alarm_e {
    ALARM_NONE,
    ALARM_RISING,
    ALARM_FALLING,
    ALARM_RISING_OR_FALLING
} alarm_startup_alarm_t;

typedef enum alarm_sample_type_e {
    RMON_ALARM_SAMPLE_TYPE_ABSOLUTE = 1,
    RMON_ALARM_SAMPLE_TYPE_DELTA
} alarm_sample_type_t;

typedef enum event_type_e {
    EVENT_TYPE_NONE = 1,
    EVENT_TYPE_LOG,
    EVENT_TYPE_TRAP,
    EVENT_TYPE_LOG_TRAP
} event_type_t;

enum rmon_entry_type_e {
    RMON_CONFIG_SYNC_ENTRY,
    RMON_ETHERSTAT_ENTRY,
    RMON_ETHERSTAT_START_ENTRY,
    RMON_IFSTAT_ENTRY,
    RMON_IFSTAT_START_ENTRY,
    RMON_IFSTAT_DATA_ENTRY,
    RMON_EVENT_ENTRY,
    RMON_LOG_ENTRY,
    RMON_ALARM_ENTRY,
    RMON_HISTORY_ENTRY,
    RMON_HISTORY_DATA_ENTRY,
    RMON_HISTORY_CTRL_ENTRY,
    RMON_HISTORY_DIRTY_ENTRY,
} rmon_entry_type_t;

typedef enum rmon_entry_status_e {
    RMON_ENTRY_VALID = 1,
    RMON_ENTRY_CREATE_REQUEST,
    RMON_ENTRY_UNDER_CREATION,
    RMON_ENTRY_INVALID
} rmon_entry_status_s;

typedef enum rmon_history_sample_e {
    RMON_STATS_RX_BYTES,
    RMON_STATS_RX_UNICAST_FRAMES,
    RMON_STATS_RX_MULTICAST_FRAMES,
    RMON_STATS_RX_BROADCAST_FRAMES,
    RMON_STATS_RX_ERROR_FRAMES,
    RMON_STATS_RX_CRC_ERROR_FRAMES,
    RMON_STATS_RX_RUNT_FRAMES,
    RMON_STATS_RX_DROP_EVENT,
    RMON_STATS_RX_GOOD_OVERSIZED_FRAMES,
    RMON_STATS_RX_FRAGMENT_FRAMES,
    RMON_STATS_RX_JABBER_FRAMES,
    RMON_STATS_TX_COLLISION_FRAMES,
    RMON_STATS_NUM_OF_INTF_ENTRIES
} rmon_history_sample_t;

/* Define Enums - End */

/* Define Structures - Start */

typedef struct
history_data_s {
    void *data;
    struct history_data_s *flink;
    struct history_data_s *blink;
} history_data_t;

typedef struct
history_table_s {
    int history_index; /* Index to tree */
    int requested;
    int interval;
    int granted;
    int samples_created_count;
    int time_left;
    void *data; /* This can be any data encapsulated with */
    int entry_status; /* In our case, always entry is valid */
    history_data_t *head_t; /* pointer to head node in CQ */
    history_data_t *last_t; /* pointer to tail node in CQ */
    history_data_t *update_t; /* pointer to actual last node updated */
    atavl_node_t history_node;
    atavl_node_t dirty_node;
} history_table_t;

typedef struct rmon_hist_ctrl_data_s {
    int if_index;
    char if_name[RMON_IF_NAME_SIZE];
    char owner_string[RMON_OWNER_STR_SIZE];
} rmon_hist_ctrl_data_t;

typedef struct rmon_ifstat_entry_s {
    int          sample_index;
    int          timestamp;
    int          if_util; /* Utilization is calculated during the span of
                             interval, for calculation, check RFC2819 */
    uint64_t     if_stats[RMON_STATS_NUM_OF_INTF_ENTRIES];
} rmon_ifstat_entry_t;

typedef struct rmon_ifstat_data_s {
    int          if_index;
    char         if_name[RMON_IF_NAME_SIZE];
    uint64_t     if_stats[SYSTEM_STATS_NUM_OF_INTF_ENTRIES];
    atavl_node_t ifstat_node;
} rmon_ifstat_data_t;

/* The below node is required to store the ifstat data at the start of
 * interval for history table indexed by hist_index
 */
typedef struct rmon_ifstat_start_entry_s {
    int8_t       flag;
    int          hist_index;
    uint64_t     if_stats[SYSTEM_STATS_NUM_OF_INTF_ENTRIES];
    atavl_node_t ifstat_start_node;
} rmon_ifstat_start_entry_t;

typedef struct rmon_lif_data_s {
    int          if_index;
    char         if_name[RMON_IF_NAME_SIZE];
    uint64_t     if_stats[SYSTEM_STATS_NUM_OF_INTF_ENTRIES];
    atavl_node_t lif_node;
} rmon_lif_data_t;

typedef struct alarm_entry_s {
    int alarm_index;
    int interval;
    int time_left; /* interval - time_spent by the timer */
    int if_index;
    int sample_type;
    int startup_alarm;
    int rising_threshold_value;
    int falling_threshold_value;
    int rising_event_index;
    int falling_event_index;
    int status;
    int last_alarm_seen;
    uint64_t last_seen_value;
    uint64_t last_absolute_value;
    char if_name[RMON_IF_NAME_SIZE];
    char owner_string[RMON_OWNER_STR_SIZE];
    char variable[RMON_OBJ_NAME_LEN];
    atavl_node_t alarm_node;
} alarm_entry_t;

typedef struct event_entry_s {
    int8_t delete_state;
    int  event_index;
    int  type;
    int  last_time_sent;
    int  status;
    char desc[RMON_EVENT_DESC_LEN];
    char community[RMON_COMMUNITY_STR_LEN];
    char owner_str[RMON_OWNER_STR_SIZE];
    atavl_node_t event_node;
} event_entry_t;

typedef struct log_entry_s {
    int  event_index;
    int  log_index;
    int  log_time; /* system uptime when log was created */
    int  alarm_index;
    int  alarm_value;
    int  alarm_sample_type;
    int  threshold_value;
    char log_desc[RMON_LOG_DESC_LEN];
    atavl_node_t log_node;
    /* Below two are for CLI displays */
    atavl_node_t log_time_node; /* Index is log_time */
    atavl_node_t log_time_event_node; /* Index is log_time and event_index */
} log_entry_t;

typedef struct alarm_ifstat_counter_entries_s {
    int  num_counters;
    int  oid_format; /* From CLI, either set OID format/string format */
    char obj_name[RMON_OBJ_NAME_LEN];
    int  counters[RMON_MAX_IFSTAT_COUNTER_PER_IF];
} alarm_ifstat_counter_entries_t;

typedef struct etherstat_entry_s {
    int8_t       delete_state;
    int          if_index;
    int          stat_index;
    char         owner_str[RMON_OWNER_STR_SIZE];
    char         if_name[RMON_IF_NAME_SIZE];
    atavl_node_t etherstat_node;
} etherstat_entry_t;

/* The node is required to store the ifstat data at ths creation of
 * etherstat entry indexed by etherstat index
 */
typedef struct rmon_etherstat_start_entry_s {
    int          if_index;
    int          stat_index;
    char         start_date[RMON_DATE_LEN];
    uint64_t     if_stats[SYSTEM_STATS_NUM_OF_INTF_ENTRIES];
    atavl_node_t etherstat_start_node;
} rmon_etherstat_start_entry_t;

/* Define Structures - End */

/* Declare Functions - Start */

/* Atavl Tree Declarations */
extern atavl_tree_t history_table_tree;
extern atavl_tree_t rmon_ifstat_data_tree;
extern atavl_tree_t rmon_lif_tree;
extern atavl_tree_t rmon_alarm_tree;
extern atavl_tree_t rmon_event_tree;
extern atavl_tree_t rmon_log_tree;
extern atavl_tree_t rmon_log_time_tree;
extern atavl_tree_t rmon_log_event_tree;
extern atavl_tree_t rmon_etherstat_tree;
extern atavl_tree_t rmon_ifstat_start_tree;
extern atavl_tree_t rmon_etherstat_start_tree;
extern atavl_tree_t history_dirty_entry_tree;

/* Tracing */
extern uint32_t rmon_trace_flag;
extern uint32_t rmon_trace_level;

/* Declartions for Etherstat */
etherstat_entry_t *rmon_get_etherstat_entry(int);
etherstat_entry_t *rmon_get_next_etherstat_entry(int);
etherstat_entry_t *rmon_next_etherstat_entry(etherstat_entry_t *);
rmon_etherstat_start_entry_t *rmon_get_etherstat_init_entry(int);
rmon_etherstat_start_entry_t *rmon_get_next_etherstat_init_entry(int);
void rmon_update_etherstat_data(uint64_t *, int);
void rmon_update_etherstat_entry(etherstat_entry_t *, etherstat_entry_t *);
void rmon_delete_etherstat_entry_by_index(int);
void rmon_delete_etherstat_entry(etherstat_entry_t *);
void rmon_etherstat_entry_db_delete();
void rmon_update_start_etherstat_entry_by_ifindex(uint32_t, uint64_t []);
int rmon_set_etherstat_entry(etherstat_entry_t *);
void rmon_delete_etherstat_entry_by_ifindex(int);

/* Declarations for History */
history_table_t *history_get_entry(int);
history_table_t *history_get_next_entry(int);
history_table_t *rmon_insert_dirty_entry(int);
history_table_t *rmon_history_entry_modify_by_dirty_entry(history_table_t *, 
                                              history_table_t *);
void rmon_delete_dirty_entry(int);
history_table_t *rmon_history_get_dirty_entry(int);

void history_init_tree();
history_table_t *history_register_table(int , int , int, int, atavl_tree_t *);
int history_node_cmp(const void *, const void *);
int dirty_node_cmp(const void *, const void *);
history_table_t *history_register(int, int, int, int, char *);
int rmon_update_if_utilization(uint64_t prev_ifstat[],
                               uint64_t curr_ifstat[],
                               history_table_t *hist_ptr,
                               int is_first_sample);
void rmon_map_update_history_ifstat_data(uint64_t dst_if_stats[], uint64_t
                                         actual_if_stats, int i);
void rmon_update_history_sample_index(history_data_t * );
void rmon_delete_history_data (history_data_t *);
void rmon_delete_history_entry_by_ifindex(int);
void rmon_history_entry_delete(history_table_t *);
void rmon_history_entry_db_delete();
void rmon_insert_history_data_entry_queue(history_table_t *, history_data_t *);
void rmon_create_history_buckets(history_table_t *, int);
int rmon_map_hw_to_rmon_ifstat_index(int);
int rmon_map_rmon_to_hw_ifstat_index(int);
void rmon_update_start_history_entry_by_ifindex(int32_t if_index, uint64_t
                                                if_stats[]);
void rmon_update_ifstat_data(uint64_t update_ifstat[],
                             history_data_t *hist_ptr, history_table_t *ptr);
void rmon_modify_map_ifstat(uint64_t dst_if_stats[], uint64_t src_if_stats[]);

/* Declarations for Alarm */
int rmon_set_alarm_entry(alarm_entry_t *);
int rmon_reset_alarm_entry(alarm_entry_t *);
int rmon_reset_alarm_entry_by_index(int);
void rmon_alarm_entry_db_delete();
void rmon_delete_alarm_entry_by_ifindex(int);
void rmon_start_alarm_update_timer();
alarm_entry_t *rmon_get_alarm_entry(int);
alarm_entry_t *rmon_get_next_alarm_entry(int);
void rmon_check_send_trap_log(uint64_t , alarm_entry_t *);

/* Declarations for Event */                         
char *rmon_get_event_type(int );
event_entry_t *rmon_get_event_entry(int);
event_entry_t *rmon_get_next_event_entry(int);
event_entry_t *rmon_next_event_entry(event_entry_t *);
event_entry_t *rmon_set_event_entry(event_entry_t *);
void rmon_event_entry_delete(event_entry_t *);
void rmon_event_entry_db_delete();

/* Declarations for Log */
log_entry_t *rmon_get_log_entry(int, int);
log_entry_t *rmon_get_next_log_entry(int, int);
log_entry_t *rmon_get_next_log_entry_by_event_id(int, int);
log_entry_t *rmon_get_next_log_entry_by_event(log_entry_t *);
log_entry_t *rmon_get_prev_log_entry(int, int);
log_entry_t *rmon_get_first_log_time_entry();
log_entry_t *rmon_get_log_time_entry(int, int, int);
log_entry_t *rmon_get_next_log_time_entry(int, int, int);
log_entry_t *rmon_get_first_log_event_entry();
log_entry_t *rmon_get_log_event_entry(int, int, int);
log_entry_t *rmon_get_next_log_event_entry(int, int, int);
log_entry_t *rmon_get_last_log_time_entry();
log_entry_t *rmon_get_prev_log_time_entry(int, int, int);
log_entry_t *rmon_prev_log_event_entry(int, int, int);
void rmon_log_entry_db_delete();

/* Declarations for ifstat */
uint64_t rmon_get_if_pkts_count(uint64_t if_stats[]);
uint64_t rmon_get_etherstat_pkts_count(uint64_t if_stats[]);
rmon_ifstat_data_t *rmon_get_ifstat_data_entry(int);
rmon_ifstat_data_t *rmon_get_next_ifstat_data_entry(int);
rmon_ifstat_start_entry_t *rmon_get_ifstat_init_entry(int);
rmon_ifstat_start_entry_t *rmon_get_next_ifstat_init_entry(int);
int rmon_update_ifstat_data_entry(rmon_ifstat_data_t *, int);
void rmon_insert_ifstat_init_entry(int, rmon_ifstat_data_t *);
extern alarm_ifstat_counter_entries_t alarm_ifstat_counter[];
extern alarm_ifstat_counter_entries_t *rmon_find_counter_entry(char *);
rmon_ifstat_data_t *rmon_get_or_new_ifstat_data_entry(int);

/* Misc Declarations */
char *rmon_get_ifname_by_ifindex(int);
void rmon_get_obj_name_by_oid(char *variable, char *text, int *);
void rmon_mib_oid_init();
void rmon_del_reference_by_ifindex(uint32_t);

/* Declare Functions - End */

#endif /* __RMON_DEFS_H__ */
