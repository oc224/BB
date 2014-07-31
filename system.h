#define FAIL -1
#define SUCCESS 0
#define DEBUG_MSG_PATH "home/root/log/debug.txt"
#define SYS_CFG_PATH "/home/root/config/this_node.txt"
#define DEFAULT_SCRIPT "/home/root/config/schedule.txt"
#define DEFAULT_DEPLOY "/home/root/config/deploy.txt"
#define NAME_SIZE 16

typedef struct{
	char *name;
}node_cfg;

typedef struct{
	char name[NAME_SIZE];
	char w_add[NAME_SIZE];
	char w_net[NAME_SIZE];
	char a_add[NAME_SIZE];
	char ppp_ip[NAME_SIZE];
}node;

int system_node_lookup(const char *,node * info);
int system_cfg_read();
void system_cfg_show();
int system_msg_dump(char *);
