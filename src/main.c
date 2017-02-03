/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbelouni <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/02/01 17:16:24 by nbelouni          #+#    #+#             */
/*   Updated: 2017/02/03 00:59:22 by alallema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42sh.h"

int main(int argc, char **argv, char **envp)
{
	(void)argc;
	(void)argv;
	(void)envp;

	t_buf	*buf;

	signal(SIGWINCH, get_sigwinch);
	signal(SIGINT, get_sigint);
	if (!(buf = init_buf()))
		return (-1);
	init_curs();
	while (read_line(buf) >= 0)
	{
		close_termios();
		ft_putstr_fd(buf->line, 1);
		X('\n');
		ft_putnbr_fd(buf->size, 1);
		ft_bzero(buf->line, (size_t)buf->size);
		buf->size = 0;
		clean_pos_curs();
	}
	close_termios();
	destroy_curs();
	free_buf(buf);
	return (0);
}
