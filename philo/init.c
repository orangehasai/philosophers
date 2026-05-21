/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:55:57 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 15:46:45 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

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

static void	init_philos(t_rules *rules)
{
	int	i;

	i = 0;
	while (i < rules->num_philo)
	{
		init_philo(rules, i);
		i++;
	}
}

int	init_data(t_rules *rules)
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
		free(rules->forks);
		rules->forks = NULL;
		return (1);
	}
	rules->start_us = now_us();
	init_philos(rules);
	return (0);
}
