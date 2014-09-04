typedef struct
{
float lon;
float lat;
float knot;
float alt;
int hh;
int mm;
int ss;
int day;
int month;
int year;
int no_sat;
}gps_info;

extern gps_info gps;

int gps_update();
void gps_show();
void gps_log();
