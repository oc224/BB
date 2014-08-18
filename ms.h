#define PREP "req"
#define ACK "ack"
#define MILI 1000
#define WAIT_THEN_PLAY (100*MILI)
#define SLEEP_BEFORE_RECORD (700*MILI)
#define SLEEP_AFTER_SYNC 2
#define REMOTE_TIMEOUT 10000

typedef char bool;

typedef enum{
	NONE,
        TALK,//recipricol transmission, ok
	CONVERSATION,
	QUICK,
	MSPLAY,//local modem play
	MSRECORD,//local modem record
	SYNCALL,//sync remote & local modem
	HELP,//show help msg
	UPLOAD,//upload local files
	SEND_REMOTE,//send msg to remote modems
	MSG_SHOW,//show msg & msg_remote
	WAIT_REMOTE,//wait remote msg
	CLEAR_FFS,//clear local ffs
	STATUS,
	GPSLOG
}cmd_type;

typedef struct{
cmd_type type;
bool isremote;
}cmd;



int master_talk();
int slave_talk();
int master_con();
int slave_con();
int master_quick();
int slave_quick();
int master_sync();
int slave_sync();
void help();
int play(const char*);
int record(const char*);
int upload(const char*);
int msg_send();
int wait_remote();
