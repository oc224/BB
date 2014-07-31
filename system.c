#include "system.h"
#include <stdio.h>
#include "string.h"
#define BUFSIZE 128
#define NAMESIZE
node_cfg t_node;

int system_cfg_read(){
	//see cfg of this node
FILE *fp;
char name[BUFSIZE];
int rt=SUCCESS;
memset(name,0,BUFSIZE);
fp=fopen(SYS_CFG_PATH,"r");
if (fp==NULL){
	printf("fail to read this_node.txt  file\n");
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
int system_node_lookup(const char *name,node * info){
	/*read deploy*/
	FILE *fp;
	char buf[BUFSIZE];
	fp=fopen(DEFAULT_DEPLOY,"r");
	if(fp==NULL){
		fprintf(stderr,"fail to read deploy.txt \n");
	}
	printf("debug\n");
	while(fgets(buf,BUFSIZE,fp)!=NULL){
		if (buf[0]=='#')continue;
		if (strlen(buf)<2)continue;
		//Anderson	11	11	11	10.0.0.11
		if (strcasestr(buf,name)==NULL)continue;
		if (sscanf(buf,"%s %s %s %s %s ",info->name,info->w_add,info->w_net,info->a_add,info->ppp_ip)<5){
			fprintf(stderr,"fail to read node info\n");
		}else{
			return SUCCESS;
		}
	}
	return FAIL;
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
