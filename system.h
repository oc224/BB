#define FAIL -1
#define SUCCESS 0
#define DEBUG_MSG_PATH "/home/root/log/debug.txt"
#define TNODE_PATH "/home/root/config/this_node.txt"
#define DEPLOY_PATH "/home/root/config/deploy.txt"
#define DEFAULT_SCRIPT "/home/root/config/schedule.txt"

#define NAME_SIZE 20
#define N_NODE_MAX 4
typedef enum{MASTER,SLAVE}opmode;

typedef struct{
	char name[NAME_SIZE];
	char w_add[NAME_SIZE];
	char w_net[NAME_SIZE];
	char a_add[NAME_SIZE];
	char ppp_ip[NAME_SIZE];
	char tx_fname[NAME_SIZE];
}node;

typedef struct{
	char *name;
	node *nodes[N_NODE_MAX];
	node *this_node; /*nodes[this_node] point to this node*/
	int N_node;
	opmode mode;
}this_node;

extern this_node t_node;
int system_cfg_read();
node* system_cfg_find(const char*);
void system_cfg_show();
int system_msg_dump(char *);
