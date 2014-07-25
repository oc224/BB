#include "system.h"
#include <stdio.h>

node_cfg t_node;
int id_read(){
	//see id of this node
FILE *fp;
int rt=SUCCESS;
fp=fopen("./config/this_node.txt","r");
if (fp==NULL){
	printf("fail to read this_node.txt config file\n");
	return FAIL;
}
if (fscanf(fp,"name %s",t_node.name)<1){
	printf("fail to read this_node.txt (name)\n");
	rt=FAIL;
}
fclose(fp);
return rt;
}

int debug_msg_dump(char *msg){
	FILE *fp;
	fp=fopen(DEBUG_MSG_PATH,"wa");
	if (fp==NULL){
		printf("fail to open debug.txt\n");
		return FAIL;
	}
	fputs(msg,fp);
	fclose(fp);
	return SUCCESS;
}
