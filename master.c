#include <stdio.h>
#include <string.h>
#define BUFSIZE 128

int id_command(char* str){
	int command;
	//printf("debug: %s\n",str);
	if (strstr(str,"exp"))command=0;
	else if (strstr(str,"prob"))command=1;
	else if (strstr(str,"status"))command=2;
	printf("debug :%d\n",command);
	return 0;
}
int main(){
int cnt=1;
char i_buf[BUFSIZE];
while (1){
fprintf(stdout,"master<%d>:",cnt);
fgets(i_buf,BUFSIZE,stdin);
id_command(i_buf);
cnt++;

}
return 0;
}
