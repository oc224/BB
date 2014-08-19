#include "gps.h"
#include <unistd.h>

int main(){
gps_update();
gps_show();
while (1){
sleep(10);
gps_log();
}
return 0;
}
