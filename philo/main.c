/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:55:59 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 19:46:13 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	main(int ac, char **av)
{
	t_rules	rules;

	if (parse_args(ac, av, &rules))
		return (1);
	if (init_data(&rules))
		return (1);
	if (rules.num_philo == 1)
		handle_one_philo(&rules.philos[0]);
	cleanup_all(&rules);
	return (0);
}
