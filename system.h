#define FAIL -1
#define SUCCESS 0
#define NODE_INFO_NAME_MAX 32
#define DEBUG_MSG_PATH "./Log/debug.txt"

typedef struct{
	char name[NODE_INFO_NAME_MAX];
}node_cfg;

int id_read();
int debug_msg_dump(char *);
