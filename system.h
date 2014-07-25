#define FAIL -1
#define SUCCESS 0
#define DEBUG_MSG_PATH "./Log/debug.txt"
#define SYS_CFG_PATH "./config/this_node.txt"

typedef struct{
	char *name;
}node_cfg;

int system_cfg_read();
void system_cfg_show();
int system_msg_dump(char *);
