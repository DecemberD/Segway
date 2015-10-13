
extern struct Pid PidCSAR;
extern struct Pid PidCSBR;
extern struct Pid PidCSAL;
extern struct Pid PidCSBL;
extern struct Pid PidSpeedR;
extern struct Pid PidSpeedL;
extern int PidExec(int, int, struct Pid*);
//extern int PidExec(int in, int setp, struct Pid* pid);