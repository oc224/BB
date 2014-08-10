#include "system.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUFSIZE 128
this_node t_node;

int system_cfg_read(){
	/*load cfg from this_node.txt & deploy.txt*/
FILE *fp;
char buf[BUFSIZE];
int rt=SUCCESS;
memset(buf,0,BUFSIZE);
/*open this_node.txt */
fp=fopen(TNODE_PATH,"r");
if (fp==NULL){
	printf("fail to read this_node.txt  file\n");
	return FAIL;
}

/*scan cfg file*/
if (fscanf(fp,"name %s",buf)<1){
	printf("fail to read this_node.txt (name)\n");
	rt=FAIL;
}
t_node.name=strdup(buf);

if (fscanf(fp,"opmode %s",buf)<1){
	printf("fail to read this_node.txt (opmode)\n");
	rt=FAIL;
}


/*close cfg file*/
fclose(fp);

/* open deploy cfg deploy.txt*/
fp=fopen(DEPLOY_PATH,"r");
if (fp==NULL){
	printf("fail to read deploy file\n");
	return FAIL;
}

/*scanf deploy cfg*/
int i=0;
while(fgets(buf,BUFSIZE,fp)!=NULL){
	if (buf[0]=='#')continue;
	if (strlen(buf)<2)continue;
// name 		w_unit_addr w_net_addr a_unit_addr ppp_ip
	//Anderson	11	11	11	10.0.0.11
	t_node.nodes[i]=malloc(sizeof(node));
	if (sscanf(buf,"%s %s %s %s %s %s",t_node.nodes[i]->name,\
t_node.nodes[i]->w_add,t_node.nodes[i]->w_net,\
t_node.nodes[i]->a_add,t_node.nodes[i]->ppp_ip,\
t_node.nodes[i]->tx_fname)==6){
		i++;
printf("node read\n");
}
}
t_node.N_node=i;
t_node.this_node=system_cfg_find(t_node.name);
/*close */
fclose(fp);
return rt;
}

void system_cfg_show(){
	//show cfg of this node
	int i;
	printf("this node info\n");
	printf("name : %s\n\n",t_node.name);
	printf("nodes info\n");
	for (i=0;i<t_node.N_node;i++)
		printf("%12s %4s %4s %4s %4s %12s\n",\
t_node.nodes[i]->name,t_node.nodes[i]->w_add,\
t_node.nodes[i]->w_net,t_node.nodes[i]->a_add,\
t_node.nodes[i]->ppp_ip,t_node.nodes[i]->tx_fname);
}

node* system_cfg_find(const char* name){
/*return node pointer that correspod to 'name' node*/
	int i;
	for(i=0;i<t_node.N_node;i++){
		if (strstr(t_node.nodes[i]->name,name)!=NULL)break;
	}

	return t_node.nodes[i];
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
