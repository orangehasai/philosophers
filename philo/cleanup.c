/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:55:55 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 17:54:59 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	cleanup_arrays(t_rules *rules)
{
	free(rules->philos);
	free(rules->forks);
	rules->philos = NULL;
	rules->forks = NULL;
}

static int	destroy_state_mutexes(t_rules *rules, int count)
{
	int	err;

	err = 0;
	while (count-- > 0)
	{
		if (pthread_mutex_destroy(&rules->philos[count].state_mutex) != 0)
			err = 1;
	}
	return (err);
}

static int	destroy_fork_mutexes(t_rules *rules, int count)
{
	int	err;

	err = 0;
	while (count-- > 0)
	{
		if (pthread_mutex_destroy(&rules->forks[count]) != 0)
			err = 1;
	}
	return (err);
}

void	cleanup_init_failure(t_rules *rules, int fork_count, int state_count)
{
	destroy_state_mutexes(rules, state_count);
	destroy_fork_mutexes(rules, fork_count);
	pthread_mutex_destroy(&rules->print_mutex);
	pthread_mutex_destroy(&rules->stop_mutex);
	cleanup_arrays(rules);
}

int	cleanup_all(t_rules *rules)
{
	int	err;

	err = 0;
	if (destroy_state_mutexes(rules, rules->num_philo))
		err = 1;
	if (destroy_fork_mutexes(rules, rules->num_philo))
		err = 1;
	if (pthread_mutex_destroy(&rules->print_mutex) != 0)
		err = 1;
	if (pthread_mutex_destroy(&rules->stop_mutex) != 0)
		err = 1;
	cleanup_arrays(rules);
	return (err);
}
