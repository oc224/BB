typedef char bool;

typedef enum{
        EMPTY,
        NONE,
        TALK,//recipricol transmission, ok
        ATALK,
        CONVERSATION,
        CONEND,
        QUICK,
        MSPLAY,//local modem play
        MSRECORD,//local modem record
        SYNCALL,//sync remote & local modem
        HELP,//show help msg
        UPLOAD,//upload local files
	ANAL,
        XCORR,
        SEND_REMOTE,//send msg to remote modems
        MSG_SHOW,//show msg & msg_remote
        STATUS,
        GPSLOG,
        RREBOOT
}cmd_type;

//Task info
typedef struct{
cmd_type type;
bool isremote;
char arg[128];
}TASK;
