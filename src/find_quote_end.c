/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_quote_end.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbelouni <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/03/10 12:49:02 by nbelouni          #+#    #+#             */
/*   Updated: 2017/03/10 22:03:40 by nbelouni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42sh.h"

int		is_end(char *s, int *i, char c)
{
	int		len;

	if (is_char(s, *i, c))
	{
		*i += 1;
		if ((len = find_next_char(s, *i, c)) < 0)
			return (FALSE);
		else
			*i += len;
	}
	return (TRUE);
}

int		btq_in_dquote(char *s, int *i, int *n_dquote)
{
	(*n_dquote) += 1;
	while (++(*i) < (int)ft_strlen(s))
	{
		if (is_end(s, i, '`') == FALSE)
			return (FALSE);
		else if (is_char(s, *i, '"'))
		{
			(*n_dquote) += 1;
			break ;
		}
	}
	if (*n_dquote % 2 == 1)
		return (FALSE);
	return (TRUE);
}

int		find_quote_end(char *s)
{
	int		i;
	int		n_dquote;

	i = -1;
	n_dquote = 0;
	while (++i < (int)ft_strlen(s))
	{
		if (is_end(s, &i, '\'') == FALSE)
			return (FALSE);
		else if (is_end(s, &i, '`') == FALSE)
			return (FALSE);
		else if (is_char(s, i, '"'))
		{
			if (btq_in_dquote(s, &i, &n_dquote))
				return (FALSE);
		}
	}
	return (TRUE);
}
