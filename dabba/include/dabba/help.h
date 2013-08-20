
#ifndef HELP_H
#define	HELP_H

struct option;

void show_usage(const struct option *opt);
int cmd_help(int argc, const char **argv);
int cmd_version(int argc, const char **argv);
void help_unknown_cmd(const char *const cmd);

#endif				/* HELP_H */
