/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   io.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llaffile <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/03/21 11:02:14 by llaffile          #+#    #+#             */
/*   Updated: 2017/03/21 16:04:30 by llaffile         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "42sh.h"
#include "io.h"

Io_p new_io()
{
	Io_p	io;

	io = malloc(sizeof(struct Io));
	bzero(io, sizeof(struct Io));
	return(io);
}

