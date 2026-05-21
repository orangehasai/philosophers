/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:04 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 22:17:59 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* TODO: Replace with stop-based waiting after T11 monitor loop. */
void	handle_one_philo(t_philo *philo)
{
	long long	deadline;

	pthread_mutex_lock(philo->left_fork);
	print_state(philo, "has taken a fork");
	deadline = philo->last_meal_us + philo->rules->time_to_die_ms * 1000LL;
	while (now_us() < deadline)
		usleep(200);
	pthread_mutex_unlock(philo->left_fork);
}

static void	stagger(t_philo *philo)
{
	if (philo->id % 2 == 0)
		usleep(500);
}

static void	*philo_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	stagger(philo);
	while (!simulation_should_stop(philo->rules))
	{
		take_forks(philo);
		eat(philo);
		put_forks(philo);
		philo_sleep(philo);
		think(philo);
	}
	return (NULL);
}

int	start_routine(t_rules *rules)
{
	int	i;

	i = 0;
	while (i < rules->num_philo)
	{
		if (pthread_create(&rules->philos[i].thread, NULL, philo_routine,
				&rules->philos[i]) != 0)
			return (1);
		i++;
	}
	return (0);
}
