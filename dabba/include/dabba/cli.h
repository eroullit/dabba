
#ifndef CLI_H
#define	CLI_H

int str2bool(const char *const str, int *const val);
int str2speed(const char *const str, uint32_t * const speed);
int str2duplex(const char *const str, int *const duplex);

char *duplex2str(const int duplex);

int str2sched_policy(const char *const policy_name);
const char *sched_policy2str(const int policy);
const char *thread_type2str(const int type);
const char *port2str(const uint8_t port);

#endif				/* CLI_H */
