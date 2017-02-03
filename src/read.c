/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alallema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/02/01 18:03:10 by alallema          #+#    #+#             */
/*   Updated: 2017/02/03 14:35:12 by nbelouni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42sh.h"
#include "read.h"

int		read_line(t_buf *buf)
{
	unsigned int	x;
	int				ret;

	x = 0;
	init_termios();
	while ((ret = read(0, (char *)&x, 4)))
	{
		E(1);X(':');E(g_curs.col);X('\n');
		if (ret < 0 || buf->size > BUFF_SIZE)
			return (-1);
		if (x == CTRL_D)
			return (-2);
		if (x > 31 && x < 127)
		{
			t_puts("im", 1);
			t_puts("ic", 1);
			ft_putchar_fd((char)x, 2);
			ft_putchar_fd((char)x, 1);
			buf->line[g_curs.win_col * g_curs.row + g_curs.col] = (char)x;
			buf->size++;
			if (g_curs.col < g_curs.win_col)
				g_curs.col++;
			else
			{
				g_curs.col = 0;
				g_curs.row++;
			}
			t_puts("ei", 1);
		E(2);X(':');E(g_curs.col);X('\n');
		}
		if (x == LEFT)
			tab_move[1](buf);
		if (x == RIGHT)
			tab_move[0](buf);
		if (x == RETR)
		{
			ft_putchar_fd((char)x, 1);
			return (0);
		}
		x = 0;
		E(3);X(':');E(g_curs.col);X('\n');
	}
	close_termios();
	return (0);
}