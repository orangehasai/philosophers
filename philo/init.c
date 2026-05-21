/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:55:57 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 18:07:16 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	alloc_arrays(t_rules *rules)
{
	rules->stop = 0;
	rules->forks = NULL;
	rules->philos = NULL;
	rules->forks = malloc(sizeof(pthread_mutex_t) * rules->num_philo);
	if (!rules->forks)
		return (1);
	rules->philos = malloc(sizeof(t_philo) * rules->num_philo);
	if (!rules->philos)
	{
		cleanup_arrays(rules);
		return (1);
	}
	return (0);
}

static int	init_global_mutexes(t_rules *rules)
{
	if (pthread_mutex_init(&rules->stop_mutex, NULL))
		return (1);
	if (pthread_mutex_init(&rules->print_mutex, NULL))
	{
		pthread_mutex_destroy(&rules->stop_mutex);
		return (1);
	}
	return (0);
}

static int	init_local_mutexes(t_rules *rules, int *fork_count,
		int *state_count)
{
	*fork_count = 0;
	*state_count = 0;
	while (*fork_count < rules->num_philo)
	{
		if (pthread_mutex_init(&rules->forks[*fork_count], NULL) != 0)
			return (1);
		(*fork_count)++;
	}
	while (*state_count < rules->num_philo)
	{
		if (pthread_mutex_init(&rules->philos[*state_count].state_mutex,
				NULL) != 0)
			return (1);
		(*state_count)++;
	}
	return (0);
}

static void	init_philo(t_rules *rules, int i)
{
	t_philo	*philo;

	philo = &rules->philos[i];
	philo->id = i + 1;
	philo->meals_eaten = 0;
	philo->last_meal_us = rules->start_us;
	philo->rules = rules;
	philo->left_fork_id = i;
	philo->right_fork_id = (i + 1) % rules->num_philo;
	philo->left_fork = &rules->forks[i];
	philo->right_fork = &rules->forks[(i + 1) % rules->num_philo];
}

int	init_data(t_rules *rules)
{
	int	fork_count;
	int	state_count;
	int	i;

	if (alloc_arrays(rules))
		return (1);
	if (init_global_mutexes(rules))
	{
		cleanup_arrays(rules);
		return (1);
	}
	if (init_local_mutexes(rules, &fork_count, &state_count))
	{
		cleanup_init_failure(rules, fork_count, state_count);
		return (1);
	}
	rules->start_us = now_us();
	i = 0;
	while (i < rules->num_philo)
	{
		init_philo(rules, i);
		i++;
	}
	return (0);
}
