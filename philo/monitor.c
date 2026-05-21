/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:01 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/22 00:06:59 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	read_philo_state(t_philo *philo, long long *last_meal_us,
		int *meals_eaten)
{
	pthread_mutex_lock(&philo->state_mutex);
	*last_meal_us = philo->last_meal_us;
	*meals_eaten = philo->meals_eaten;
	pthread_mutex_unlock(&philo->state_mutex);
}

static int	check_philo_state(t_philo *philo, int *done_count)
{
	long long	last_meal_us;
	int			meals_eaten;

	read_philo_state(philo, &last_meal_us, &meals_eaten);
	if (now_us() - last_meal_us > philo->rules->time_to_die_ms * 1000LL)
		return (1);
	if (philo->rules->must_eat_count != MUST_EAT_UNSET
		&& meals_eaten >= philo->rules->must_eat_count)
		(*done_count)++;
	return (0);
}

void	monitor(t_rules *rules)
{
	int	i;
	int	done_count;

	while (!simulation_should_stop(rules))
	{
		i = 0;
		done_count = 0;
		while (i < rules->num_philo)
		{
			if (check_philo_state(&rules->philos[i], &done_count))
			{
				announce_death(&rules->philos[i]);
				return ;
			}
			i++;
		}
		if (done_count == rules->num_philo)
		{
			set_stop_flag(rules, 1);
			return ;
		}
		usleep(200);
	}
}
