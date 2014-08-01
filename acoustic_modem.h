#define a_modem_dev_no 18
#define a_modem_serial_baudrate 115200
#define a_modem_dev_path "/dev/ttyUSB2"

typedef enum {
	NOT_SYNC, QUALIFY, SYNC
} a_modem_sync_state;
typedef struct {
	float board_temp; //ref modem manual, atv
	float dsp_bat;
	float mdm_bat; // ref modem manual mdm_battery
	float rtc_bat; // ref modem manual rtc_battery
	a_modem_sync_state sync_stae; //synchronized or ...
} a_modem;

typedef struct {
} a_network;

inline int a_modem_wait_ack(char *ack_msg, int timeout_mili);
inline int a_modem_wait_info(char *key_word, int timeout, char *info,int info_size);
inline void a_modem_clear_io_buffer();
inline void a_modem_close();
int a_modem_is_clock_Sync(int samp_interval, int N_retry);
int a_modem_ls(); // list files
int a_modem_open();
int a_modem_play(char * filename);
int a_modem_print_configs(char * filepath); // save cfg all output for future ref
int a_modem_prob(); // get atxn atrn info
int a_modem_record(int duration_mili);
int a_modem_set_deploy_configs();
int a_modem_set_devel_configs(); // set the preferable configs for devel stage, tx pwr...
int a_modem_status(); // get status (internal temp, pwr cond...) fill struct a_modem
int a_modem_sync_clock_gps();
int a_modem_sync_time_gps();
void a_modem_status_show();
int a_modem_upload_file(const char *fname);
int a_modem_play_smart(char * filename,int mili_sec);
int a_modem_msg_send(const char*msg);
inline int a_modem_gets(char* buf,int size);
inline void a_modem_puts(const char*msg);
