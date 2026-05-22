/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:04 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/22 02:54:09 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	handle_one_philo(t_philo *philo)
{
	pthread_mutex_lock(philo->left_fork);
	print_state(philo, "has taken a fork");
	while (!simulation_should_stop(philo->rules))
		usleep(200);
	pthread_mutex_unlock(philo->left_fork);
}

static void	stagger(t_philo *philo)
{
	if (philo->id % 2 == 0 || (philo->rules->num_philo % 2 == 1
			&& philo->id == 1))
		precise_sleep(philo->rules->time_to_eat_ms / 2, philo->rules);
}

static void	*philo_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	if (philo->rules->num_philo == 1)
	{
		handle_one_philo(philo);
		return (NULL);
	}
	stagger(philo);
	while (!simulation_should_stop(philo->rules))
	{
		if (take_forks(philo))
			break ;
		if (eat(philo))
		{
			put_forks(philo);
			break ;
		}
		put_forks(philo);
		finish_eating(philo);
		if (philo_sleep(philo) || think(philo))
			break ;
	}
	return (NULL);
}

int	start_routine(t_rules *rules, int *started_count)
{
	int	i;

	*started_count = 0;
	i = 0;
	while (i < rules->num_philo)
	{
		if (pthread_create(&rules->philos[i].thread, NULL, philo_routine,
				&rules->philos[i]) != 0)
			return (1);
		(*started_count)++;
		i++;
	}
	return (0);
}

int	join_threads(t_rules *rules, int started_count)
{
	int	i;

	i = 0;
	while (i < started_count)
	{
		if (pthread_join(rules->philos[i].thread, NULL) != 0)
			return (1);
		i++;
	}
	return (0);
}
