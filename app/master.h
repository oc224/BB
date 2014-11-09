#ifndef MASTER_H
#define MASTER_H
typedef char bool;

typedef enum{
        EMPTY,
        NONE,
        TALK,//recipricol transmission
        ATALK,
	PROB,
        CONVERSATION,
        CONEND,
        QUICK,
        MSPLAY,//local modem play
        MSRECORD,//local modem record
        SYNCTIME,//sync remote & local modem
        HELP,//show help msg
        UPLOAD,//upload local files
	ANAL,
	RECANAL,
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

int task_push(TASK*,cmd_type,bool,char *);
int task_pop(TASK*,TASK *);

extern TASK task_recv_master;

#endif
