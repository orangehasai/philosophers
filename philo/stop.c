/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 14:04:27 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/22 12:29:02 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	is_dead_nolock(t_philo *philo, long long now_us)
{
	return (now_us - philo->last_meal_us
		> philo->rules->time_to_die_ms * 1000LL);
}

int	is_dead(t_philo *philo)
{
	int	dead;

	pthread_mutex_lock(&philo->state_mutex);
	dead = is_dead_nolock(philo, now_us());
	pthread_mutex_unlock(&philo->state_mutex);
	return (dead);
}

int	can_take_next_action(t_philo *philo)
{
	return (!simulation_should_stop(philo->rules)
		&& !is_dead(philo));
}

int	simulation_should_stop(t_rules *rules)
{
	int	stop;

	pthread_mutex_lock(&rules->stop_mutex);
	stop = rules->stop;
	pthread_mutex_unlock(&rules->stop_mutex);
	return (stop);
}

void	set_stop_flag(t_rules *rules, int value)
{
	pthread_mutex_lock(&rules->stop_mutex);
	rules->stop = value;
	pthread_mutex_unlock(&rules->stop_mutex);
}
