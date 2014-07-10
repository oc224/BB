#define a_modem_dev_no 18
#define a_modem_serial_baudrate 115200

typedef struct{
int addr;
int board_temp;
int dsp_bat;
int mdm_bat;
int sync_stae;//synchronized or ...
//bat condition,...

}a_modem;
int a_modem_open();

int a_modem_close();

inline void a_modem_clear_FIFO();

int a_modem_play(char * filename);

int a_modem_record(char * timestamp,int duration_mili);

// sync clock
int a_modem_sync_gps();

// list files
int a_modem_ls();

// set the preferable configs for devel stage, tx pwr...
int a_modem_set_devel_configs();

int a_modem_set_deploy_configs();

// save cfg all output for future ref
int a_modem_print_configs(char * filepath);

// get status (internal temp, pwr cond...) fill struct a_modem
int a_modem_status();

// get atxn atrn info
int a_modem_prob();

int a_modem_wait_info(char *key_word,int timeout,char *info,int info_size);

int a_modem_wait_ack(char *ack_msg,int timeout_mili);
