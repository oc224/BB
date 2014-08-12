#include <stdio.h>
#include "system.h"
#include "acoustic_modem.h"


int main(){

system_cfg_read();
printf("!check this_node.txt and deploy.txt");
system_cfg_show();

return SUCCESS;
}
