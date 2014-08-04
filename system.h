#define FAIL -1
#define SUCCESS 0
#define DEBUG_MSG_PATH "home/root/log/debug.txt"
#define SYS_CFG_PATH "/home/root/config/this_node.txt"
#define SYS_DEPLOY "/home/root/config/deploy.txt"
#define DEFAULT_SCRIPT "/home/root/config/schedule.txt"

#define NAME_SIZE 16
#define N_NODE_MAX 4
typedef enum{MASTER,SLAVE}opmode;
typedef struct{
	char name[NAME_SIZE];
	char w_add[NAME_SIZE];
	char w_net[NAME_SIZE];
	char a_add[NAME_SIZE];
	char ppp_ip[NAME_SIZE];
}node;

typedef struct{
	char *name;
	node *nodes[N_NODE_MAX];
	int N_node;
	opmode mode;
}this_node;



int system_cfg_read();
int system_cfg_find(const char* name);
void system_cfg_show();
int system_msg_dump(char *);
