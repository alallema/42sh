/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_jobs.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llaffile <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/03/17 18:15:02 by llaffile          #+#    #+#             */
/*   Updated: 2017/04/20 15:50:16 by llaffile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42sh.h"
#include "io.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


void	sigchldhandler(int sig)
{
	(void)sig;
}

extern	t_list	*job_list;
int last = 0;

t_job	*find_job(pid_t pgid)
{
	t_job		*j;
	t_list		*ptr;

	ptr = job_list;
	while (ptr)
	{
		j = ptr->content;
		if (j->pgid == pgid)
			return (j);
		ptr = ptr->next;
	}
	return (NULL);
}

/*
** Return true if all processes in the job have stopped or completed
*/

int	job_is_stopped(t_job *j)
{
	t_process_p	p;
	t_list		*ptr;

	ptr = j->wait_process_list;
	while (ptr)
	{
		p = ptr->content;
		if (!p->completed && !p->stopped)
			return (0);
		ptr = ptr->next;
	}
	return (1);
}

/*
** Return true if all processes in the job have completed.
*/

int	job_is_completed(t_job *j)
{
	t_process_p	p;
	t_list		*ptr;

	ptr = j->wait_process_list;
	while (ptr)
	{
		p = ptr->content;
		if (!p->completed)
			return (0);
		ptr = ptr->next;
	}
	return (1);
}

/*
** Store the status of the process pid that was returned by waitpid.
** Return 0 if all went well, nonzero otherwise.
*/

t_process_p		get_process_by_pid(pid_t pid)
{
	t_list		*ptr_job;
	t_list		*ptr_process;
	t_job		*j;
	t_process_p	p;

	ptr_job = job_list;
	while (ptr_job)
	{
		j = ptr_job->content;
		ptr_process = j->wait_process_list;
		while (ptr_process)
		{
			p = ptr_process->content;
			if (p->pid == pid)
				return (p);
			ptr_process = ptr_process->next;
		}
		ptr_job = ptr_job->next;
	}
	return (NULL);
}

t_job	*get_job_from_pid(pid_t pid)
{
	t_list		*ptr_job;
	t_list		*ptr_process;
	t_job		*j;
	t_process_p	p;

	ptr_job = job_list;
	while (ptr_job)
	{
		j = ptr_job->content;
		ptr_process = j->wait_process_list;
		while (ptr_process)
		{
			p = ptr_process->content;
			if (p->pid == pid)
				return (j);
			ptr_process = ptr_process->next;
		}
		ptr_job = ptr_job->next;
	}
	return (NULL);
}

int	mark_process_status(pid_t pid, int status)
{
	t_process_p	p;

	dprintf(2, "<%s><%d> - last <%d> && wai <%d> and pid <%d>\n", __func__, __LINE__, last, getpid(), pid);
	if (pid > 0)
	{
		if ((p = get_process_by_pid(pid)))
		{
			p->status = status;
			get_job_from_pid(pid)->status = status;
			if (WIFSTOPPED(status))
			{
				p->stopped = 1;
				last = WSTOPSIG(status);
			}
			else
			{
				p->completed = 1;
				last = WEXITSTATUS(status);
				if (WIFSIGNALED(status))
					fprintf (stderr, "%d: Terminated by signal %d.\n",
							(int) pid, WTERMSIG(p->status));
			}

		}
					return (0);
//		else
//			return (fprintf (stderr, "No child process %d and %d.\n", pid, getpid()), -1);
	}
	else if (pid == 0 || errno == ECHILD)
		return (-1);
	else
		return (perror("waitpid"), -1);
}

/*
** Check for processes that have status information available,without blocking.
*/

void	update_status(void)
{
	int		status;
	pid_t	pid;

	while (true)
	{
		pid = waitpid(WAIT_ANY, &status, WUNTRACED | WNOHANG);
		dprintf(2, "<%s> - pid <%d> and wpid <%d>\n", __func__, getpid(), pid);
		if (mark_process_status(pid, status))
			break ;
	}
	dprintf(2, "<%s -- out> - pid <%d> and wpid <%d>\n", __func__, getpid(), pid);
}

/*
** Check for processes that have status information available,
** blocking until all processes in the given job have reported.
*/

void	wait_for_job(t_job *j)
{
	int			status;
	pid_t		pid;
	sigset_t	set;
	sigset_t	oset;

	block_signal(SIGCHLD, &set, &oset);
//	signal (SIGCHLD, SIG_DFL);
	j->notified = j->notified;
	while (!j->notified)
	{
		pid = waitpid(-1, &status, WUNTRACED);// | WNOHANG);
		dprintf(2, "<%s><%d> - pid <%d> and job : <%s> wpid <%d>\n", __func__,__LINE__, getpid(), j->command, pid);
		if (mark_process_status(pid, status) || job_is_stopped(j)
			|| job_is_completed(j))
			break ;
	}
	dprintf(2, "<%s -- out> - pid <%d> and job : <%s> wpid <%d> notif <%d>\n", __func__, getpid(), j->command, pid, j->notified);
	unblock_signal(&oset);
}

/*
** Format information about job status for the user to look at.
*/

void	format_job_info(t_job *j, const char *status)
{
	fprintf(stderr, "%ld (%s): %s <%d>\n", (long)j->pgid, status, j->command, getpid());
}


extern t_job *last_job;
/*
** Notify the user about stopped or terminated jobs.
** Delete terminated jobs from the active job list.
*/

void		do_job_notification(void)
{
	t_job	*j;
	t_list	**ptr;

	update_status();
	ptr = &job_list;
	while (*ptr)
	{
		j = (*ptr)->content;
		dprintf(2, "<%s> - pid <%d> and job : <%s> pgid <%d> notif <%d>\n", __func__, getpid(), j->command, j->pgid, j->notified);
		if (job_is_completed(j))
		{
			format_job_info(j, "completed");
			if (last_job == j)
				last_job = NULL;
			dprintf(2, "did delete <%s><%d>\n", j->command, getpid());
			delete_job(POP(ptr));
			continue ;
		}
		else if (job_is_stopped(j) && !j->notified)
		{
			format_job_info(j, "stopped");
			j->notified = 1;
		}
		if (*ptr)
		{
			ptr = &(*ptr)->next;
			dprintf(2, "<%s><%p>\n", __func__, *ptr);
		}
	}
}


t_job			*get_last_job(void)
{
	if (last_job == NULL && job_list)
		last_job = job_list->content;
	return (last_job);
}

static void		put_job_in_background(t_job *j, int cont)
{
	last_job = j;
	if (cont)
		if (kill(-j->pgid, SIGCONT) < 0)
			perror("kill (SIGCONT)");
}

static void		put_job_in_foreground(t_job *j, int cont)
{
	if (!(j->flag & WAIT))
		return ;
	last_job = j;
	tcsetpgrp(g_sh_tty, j->pgid);
	if (cont)
	{
//		tcsetattr (g_sh_tty, TCSADRAIN, &j->tmodes);
		if (kill(-j->pgid, SIGCONT) < 0)
			perror("kill (SIGCONT)");
	}
	wait_for_job(j);
//	update_status();
	tcsetpgrp(g_sh_tty, g_sh_pgid);
//	tcgetattr (g_sh_tty, &j->tmodes);
//	tcsetattr (g_sh_tty, TCSADRAIN, &shell_tmodes);
}

void			mark_job_as_running(t_job *j)
{
	t_process_p	p;
	t_list		*ptr;

	ptr = j->wait_process_list;
	while (ptr)
	{
		p = ptr->content;
		p->stopped = 0;
		ptr = ptr->next;
	}
	j->notified = 0;
}

/*
** Continue the job J.
*/

void	continue_job(t_job *j, int foreground)
{
	mark_job_as_running(j);
	if (foreground)
		put_job_in_foreground(j, 1);
	else
		put_job_in_background(j, 1);
}

extern sig_t	*g_originals;

sig_t	*getOriginals();

void	launch_process(t_process_p process, int dofork)
{
	list_iter(process->io_list, (void *)apply_redir);
	if (dofork)
	{
		restore_originals_handler();
//		signal(SIGTTOU, SIG_IGN);
	}
	ft_check_exec(&process->argv);
	if (dofork)
		exit(1);
	else
		process->completed = 1;
}

t_node_p	iter_in_order(t_node_p ptr, t_list **stock)
{
	while (ptr || *stock)
	{
		if (ptr)
		{
			PUSH(stock, ptr);
			ptr = ptr->left;
		}
		else
			return (POP(stock));
	}
	return (NULL);
}

void	apply_redir(t_io *io)
{
	int		pipefd[2];

	if (io->flag & OPEN)
	{
		if (io->flag & CLOSE && access(io->str, X_OK) == -1)
			io->dup_src = open(io->str, io->mode, DEF_FILE);
		if (io->dup_src < 0)
		{
			fputs("42sh: No such file or directory (a placer dans les error)\n", stderr);
			exit(1);
		}
	}
	if (io->flag & WRITE)
	{
		pipe(pipefd);
		io->dup_src = pipefd[0];
		write(pipefd[1], io->str, ft_strlen(io->str));
		close(pipefd[1]);
	}
	if (io->flag & DUP)
	{
		if (fcntl(io->dup_src, F_GETFL) < 0/* && errno == EBADF (Bad file descriptor)*/)
		{
			fputs("42sh: Bad file descriptor (a placer dans les error)\n", stderr);
			exit(1);
		}
		else
			dup2(io->dup_src, io->dup_target);
	}
	if (io->flag & CLOSE && io->flag ^ WRITE)
		close(io->dup_src);
}

int		do_pipe(t_process_p p1, t_process_p p2, int *io_pipe)
{
	t_io	*io_in[2];
	t_io	*io_out[2];

	if (pipe(io_pipe) == -1)
	{
		perror("pipe");
		exit(1);
	}
	io_in[0] = new_io();
	io_in[1] = new_io();
	io_out[0] = new_io();
	io_out[1] = new_io();
	io_in[0]->tab_fd[0] = dup(STDOUT_FILENO);
	io_out[0]->tab_fd[1] = dup(STDIN_FILENO);
	io_in[0]->flag = DUP | CLOSE;
	io_in[1]->flag = CLOSE;
	io_out[0]->flag = DUP | CLOSE;
	io_out[1]->flag = CLOSE;
	io_in[0]->dup_src = io_pipe[1];
	io_in[0]->dup_target = STDOUT_FILENO;
	io_in[1]->dup_src = io_pipe[0];
	io_out[0]->dup_src = io_pipe[0];
	io_out[0]->dup_target = STDIN_FILENO;
	io_out[1]->dup_src = io_pipe[1];
	PUSH(&(p1->io_list), io_in[0]);
	PUSH(&(p1->io_list), io_in[1]);
	PUSH(&(p2->io_list), io_out[0]);
	PUSH(&(p2->io_list), io_out[1]);
	return (io_pipe[1]);
}

void	give_term(int pgid, int foreground)
{
	sigset_t	set;

	sigemptyset(&set);
	sigaddset(&set, SIGTTOU);
	sigaddset(&set, SIGTTIN);
	sigaddset(&set, SIGTSTP);
	sigaddset(&set, SIGCHLD);
	sigemptyset(&set);
	if (foreground)
		tcsetpgrp(g_sh_tty, pgid);
}

int		make_children(int *pgid, int foreground)
{
	int	pid;
	int	fpid;

	pid = fork();
	if (pid == 0)
	{
//		restore_originals_handler();
		fpid = getpid();
		if (*pgid == 0) *pgid = fpid;
		setpgid(fpid, *pgid);
		give_term(*pgid, foreground);
	}
	else if (pid < 0)
	{
		perror("fork");
		exit(1);
	}
	else
	{
//		p->pid = pid;
		if (*pgid == 0) *pgid = pid;
		setpgid(pid, *pgid);
	}
	dprintf(2, "<%s><%d> - last <%d> && getpid <%d> and pid <%d>\n",__func__,  __LINE__, last, getpid(), pid);
	return (pid);
}

void	exec_simple_command(t_process_p p, int fg, int dofork, int *pgid)
{
	if (dofork)
		if ((p->pid = make_children(pgid, fg)))
			return ;
	launch_process(p, dofork);
}

int		shouldfork(t_job *j, t_list *pipeline)
{
	int			dofork;
	t_process_p	p;

	dofork = 0;
	p = pipeline->content;
	if (j->foreground == 0 || pipeline->next || !(p->flag & BUILTIN))
	{
		dofork = 1;
		j->flag = WAIT;
	}
	return (dofork);
}

void	do_pipeline(t_job *job, t_list *pipeline)
{
	int			io_pipe[2];
	int			in;
	int			out;
	int			dofork = 0;
	int			pgid;

	in = STDIN_FILENO;
	pgid = job->pgid;
	dofork |= shouldfork(job, pipeline);
	while (pipeline)
	{
		dprintf(2, "<%s><%d> - last <%d> && wai <%d> and job <%s>\n",__func__,  __LINE__, last, getpid(), job->command);
		out = (pipeline->next)? do_pipe(pipeline->content, pipeline->next->content, io_pipe) : STDOUT_FILENO;
		exec_simple_command(pipeline->content, job->foreground, dofork, &pgid);
		list_iter_int(((t_process_p)pipeline->content)->io_list, (void *)restore_fd, dofork);
		if (out != STDOUT_FILENO)
			close(out);
		if (in != STDIN_FILENO)
			close(in);
		in = io_pipe[0];
		delete_list(&(((t_process_p)pipeline->content)->io_list), &free);
		insert_link_bottom(&job->wait_process_list, new_link(memcpy(malloc(pipeline->content_size), pipeline->content, pipeline->content_size), pipeline->content_size));
		pipeline = pipeline->next;
	}
}
t_process_p	new_process(char **argv);
//extern int gc;
//			dprintf(2, "<%s><%d> - current <%p> && ifand <%d> and ifor right <%d>\n", __func__, __LINE__, current, ((t_condition_if_p)current->data)->type == IF_AND, ((t_condition_if_p)current->right->data)->type == IF_OR);
void	launch_job(t_job *j)
{
	t_node_p	current;
	t_list		*stack;

	insert_link_bottom(&job_list, new_link(j, sizeof(*j)));
	current = j->process_tree;
	stack = NULL;
	dprintf(2, "<%s><%d> - last <%d> && wai <%d> and job <%s>\n",__func__,  __LINE__, last, getpid(), j->command);
	if (!j->foreground)
	{
		int pid;
		if ((pid = make_children(&j->pgid, 0)))
		{
			PUSH(&(j->wait_process_list), new_process(NULL));
			((t_process_p)TOP(j->wait_process_list))->pid = pid;
			return ;
		}
	}
	while ((current = iter_in_order(current, &stack)))
	{
		dprintf(2, "<%s><%d> - last <%d> && wai <%d> and job <%s>\n",__func__,  __LINE__, last, getpid(), j->command);
		if (current->type == IF){
			current = ((((t_condition_if_p)current->data)->type == IF_OR && last) || (((t_condition_if_p)current->data)->type == IF_AND && !last)) ? current->right : NULL;
		}
		else
		{
			dprintf(2, "<%s><%d> - last <%d> && wai <%d> and job <%s>\n",__func__,  __LINE__, last, getpid(), j->command);
			do_pipeline(j, current->data);
			current = current->right;
			if (j->foreground)
				put_job_in_foreground(j, 0);
			else
				wait_for_job(j);
		}
	}
	if (j->foreground)
//		put_job_in_foreground(j, 0);
		update_status();
	else
		//put_job_in_background(j, 0);
		exit(0);
		dprintf(2, "<%s><%d> - last <%d> && wai <%d> and job <%p>\n",__func__,  __LINE__, last, getpid(), j);
	do_job_notification();
		dprintf(2, "<%s><%d> - last <%d> && wai <%d> and job <%p>\n",__func__,  __LINE__, last, getpid(), j);
}
