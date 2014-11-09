#ifndef MS_H
#define MS_H
#define PREP "req"
#define ACK "ack"
#define MILI 1000
#define WAIT_THEN_PLAY (100*MILI)
#define SLEEP_BEFORE_RECORD (700*MILI)
#define SLEEP_AFTER_SYNC 2
#define REMOTE_TIMEOUT 10000


int master_talk();
int slave_talk();
int master_atalk();
int slave_atalk();
int master_con();
int slave_con();
int master_conend();
int slave_conend();
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
int master_rreboot();
int slave_rreboot();
#endif
