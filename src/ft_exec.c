/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exec.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alallema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/02/15 13:08:28 by alallema          #+#    #+#             */
/*   Updated: 2017/03/08 13:11:00 by alallema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "exec.h"

static char		*ft_cut_path(char **s, char *av)
{
	char	*s1;
	char	*s2;
	int		i;

	i = 0;
	s1 = *s;
	while (s1[i] != ':')
		i++;
	s2 = ft_memalloc(sizeof(char *) * (i + 2 + ft_strlen(av)));
	s2 = ft_strncpy(s2, s1, i);
	ft_strncpy(&s2[i], "/", 1);
	ft_strncpy(&s2[i + 1], av, ft_strlen(av));
	if (s1[i] == ':')
		i++;
	s1 = ft_strdup(&s1[i]);
	free(*s);
	*s = s1;
	return (s2);
}

/*
** fonction d'execution des commandes via execve
** s-> path a changer avec l'env
*/
int			ft_check_built(char **args)
{
	int		ret;

	ret = TRUE;
	if (args != NULL && args[0] != NULL)
	{
		if (ft_strcmp(args[0], "exit") == FALSE)
			ret = FALSE;
		else if (ft_strcmp(args[0], "cd") == FALSE)
			ret = FALSE;
		else if ((ret = ft_strcmp(args[0], "env")) == FALSE)
			ret = FALSE;
		else if ((ret = ft_strcmp(args[0], "setenv")) == FALSE)
			ret = FALSE;
		else if ((ret = ft_strcmp(args[0], "unsetenv")) == FALSE)
			ret = FALSE;
		else if ((ret = ft_strcmp(args[0], "echo")) == FALSE)
			ret = FALSE;
		return (ret);
	}
	return (FALSE);
}

int			ft_check_built_fork(t_lst *env, char **args)
{
	int		ret;

	ret = TRUE;
	if (args != NULL && args[0] != NULL)
	{
		if ((ret = ft_strcmp(args[0], "exit")) == FALSE)
			ft_builtin_exit(env, args[0], args + 1);
		else if ((ret = ft_strcmp(args[0], "env")) == FALSE)
			ft_builtin_env(env, &args[1]);
		else if ((ret = ft_strcmp(args[0], "setenv")) == FALSE)
			ft_builtin_setenv(env, args[0], args + 1);
		else if ((ret = ft_strcmp(args[0], "unsetenv")) == FALSE)
			ft_builtin_unsetenv(env, args[0], &args[1]);
		else if ((ret = ft_strcmp(args[0], "echo")) == FALSE)
			ft_builtin_echo(env, args[0], args + 1);
		else if ((ret = ft_strcmp(args[0], "cd")) == FALSE)
			ft_builtin_cd(env, args[0], args + 1);
		return (ret);
	}
	return (FALSE);
}

void			ft_exec(char **av, t_lst *env)
{
	struct stat	st;
	char		*s;
	char		*s2;
	char		**envp;

	envp = ft_env_to_tab(env);
	s = ft_strdup("/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/opt/X11/bin:/usr/local/munki:/Users/alallema/bin:/Users/alallema/.rvm/bin");
	while (s)
	{
		s2 = ft_cut_path(&s, av[0]);
		if (lstat(av[0], &st) == 0 && st.st_mode & S_IXUSR)
			execve(av[0], av, NULL);
		if (lstat(s2, &st) == 0 && st.st_mode & S_IXUSR)
			execve(s2, av, NULL);
		if (!ft_strchr(s, ':'))
		{
			if (lstat(av[0], &st) == 0 && st.st_mode & S_IXUSR)
				ft_putstr_fd("42sh: exec format error: ", 2);
			else if (lstat(av[0], &st) == 0)
				ft_putstr_fd("42sh: permission denied: ", 2);
			else if (!ft_strchr(s2, ':'))
			{
				ft_putstr_fd("42sh: command not found: ", 2);
				ft_putendl_fd(av[0], 2);
				exit (127);
			}
			else
				ft_putstr_fd("42sh: no such file or directory: ", 2);
			ft_putendl_fd(av[0], 2);
			free(s);
			s = NULL;
			exit(1);
		}
	}
}

int		ft_check_exec(char **cmd, t_lst *env)
{
	int		ret;

	ret = TRUE;
	if ((ret = ft_check_built_fork(env, cmd)) != 0)
		ft_exec(cmd, env);
	PUT2("\n_________exec______________\n");
	PUT2(cmd[0]);
	return (ret);
}
