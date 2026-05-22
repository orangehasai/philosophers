/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:01 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/22 03:08:16 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	check_philo_state(t_philo *philo, int *done_count)
{
	long long	now;
	int			is_dead;

	pthread_mutex_lock(&philo->state_mutex);
	now = now_us();
	is_dead = is_dead_nolock(philo, now);
	if (philo->rules->must_eat_count != MUST_EAT_UNSET
		&& philo->meals_eaten >= philo->rules->must_eat_count)
		(*done_count)++;
	pthread_mutex_unlock(&philo->state_mutex);
	return (is_dead);
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
