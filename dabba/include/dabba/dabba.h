/**
 * \file dabba.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef DABBA_H
#define	DABBA_H

/**
 * \brief Dabba command structure
 */

struct cmd_struct {
	const char *cmd; /**< Pointer to the command string */
	int (*fn) (int, const char **);	/**< Command function pointer */
};

int cmd_run_action(const struct cmd_struct *cmd, const size_t cmd_len,
		   int argc, const char **argv);
int cmd_run_command(const struct cmd_struct *cmd, const size_t cmd_len,
		    int argc, const char **argv);

#endif				/* DABBA_H */
