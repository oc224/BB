#include "system.h"
#include <stdio.h>
#include "string.h"
node_cfg t_node;

int system_cfg_read(){
	//see cfg of this node
FILE *fp;
char name[32];
memset(name,0,32);
int rt=SUCCESS;
fp=fopen(SYS_CFG_PATH,"r");
if (fp==NULL){
	printf("fail to read this_node.txt config file\n");
	rt=FAIL;
}
if (fscanf(fp,"name %s",name)<1){
	printf("fail to read this_node.txt (name)\n");
	rt=FAIL;
}
t_node.name=strdup(name);
fclose(fp);
return rt;
}

void system_cfg_show(){
	//show cfg of this node
	printf("this node info\n");
	printf("name : %s\n",t_node.name);
}

int system_msg_dump(char *msg){
	//dump debug msg to text file
	FILE *fp;
	fp=fopen(DEBUG_MSG_PATH,"a");
	if (fp==NULL){
		printf("fail to open debug.txt\n");
		return FAIL;
	}
	fputs(msg,fp);
	fputs("\n",fp);
	fclose(fp);
	return SUCCESS;
}
