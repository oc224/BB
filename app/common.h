#ifndef COMMAN_H
#define COMMAN_H

#define FAIL -1
#define SUCCESS 1

#define PATH_AMODEM "/root/log/AMODEM.TXT"
#define PATH_TX "/root/log/TXLOG.TXT"
#define PATH_RX "/root/log/RXLOG.TXT"
#define PATH_RAW_DATA "/root/raw_data"
#define CFG_DEPLOY "/root/config/modem_cfg_deploy.txt"
#define CFG_DEVEL "/root/config/modem_cfg_devel.txt"
#define TX_DEFAULT "mseq10_T1_l1"
#define PATH_TX_SIGNAL "/root/tx/"
#define ERR_PRINT(msg) fprintf(stderr,"%s, %s",__func__,msg);



#endif
