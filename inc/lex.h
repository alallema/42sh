/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lex.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alallema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/02/11 12:30:14 by alallema          #+#    #+#             */
/*   Updated: 2017/02/15 22:41:54 by nbelouni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42sh.h"

typedef struct		s_tok
{
	int				tok;
}					t_tok;
/*
typedef struct		s_cmd
{
	int				token;
	char			**cmd;
}					s_cmd;
*/
typedef struct		s_tree
{
	int				token;
	char			**cmd;
	struct s_tree	*left;
	struct s_tree	*right;
}					t_tree;

enum				e_token2
{
	O_BRACKET,	// "("
	C_BRACKET,	// ")"
	O_BRACE, 	// "{ "
	C_BRACE, 	// ";}"
	OBT_QUOTE,	// "$("
	CBT_QUOTE,	// ")" -> == C_BRACKET
	COMMENT		// "#"
};

/*
**	return == len de la chaine a strsub -> strsub(s, i, is_*());
*/
int		is_group(char *s, int i);
int		is_separator(char *s, int i);
int		is_redirection(char *s, int i);
int		is_btquote(char *s, int i);
int		is_dquote(char *s, int i);
int		is_squote(char *s, int i);
int		is_new_btquote(char *s, int i);

int		find_btquote_end(char *s, int i);
int		find_dquote_end(char *s, int i);
int		find_squote_end(char *s, int i);
int		find_group_end(char *s, int i, int token);
int		find_dollar(char *s, int i);
int		find_new_btquote_end(char *s, int i);

int		lex_buf(char *s, t_word **lst);

/*
void	ft_cmdadd(t_cmd **alst, t_cmd *new);
void	ft_cmdclear(t_cmd **cmd);
void	ft_cmdestroy(t_cmd **begin);
t_cmd	*ft_cmdnew(int token, char *cmd);
void	ft_cmdpush(t_cmd **begin, t_cmd *new);
*/
/*
 * . pour les ouvrants/fermants, compter 2 motifs differents
 *
 * Renvoie une portion du buffer separee par group && separators
 * . appelle is_group(), is_closed()
 * int		get_complete_group(char **group);
 *
 * Renvoie une portion du buffer separee par group && separators
 * . appelle is_group(), is_closed() et is_separator()
 * char		*get_complete_cmd();
 *
 * renvoie le type d'une portion d'une commande (cmd || arg || redirection)
 * complete et set *token
 * e_type	get_args(char **token);
 *
 * t_cmd	*set_cmd();
 *
 * lst functions;
 *
 * Quand tout ca est fait, GROS CHECK :
 * int		can_parse();
 * if (TRUE)
 * 	return (lst);
 * return (NULL);
*/
