/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:55:59 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/22 00:20:10 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	main(int ac, char **av)
{
	t_rules	rules;
	int		started_count;

	if (parse_args(ac, av, &rules))
		return (1);
	if (init_data(&rules))
		return (1);
	if (start_routine(&rules, &started_count))
	{
		set_stop_flag(&rules, 1);
		if (join_threads(&rules, started_count))
			return (1);
		if (cleanup_all(&rules))
			return (1);
		return (1);
	}
	monitor(&rules);
	if (join_threads(&rules, started_count))
		return (1);
	if (cleanup_all(&rules))
		return (1);
	return (0);
}
