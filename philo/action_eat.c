/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   action_eat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 22:11:25 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/22 12:30:06 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	begin_eating(t_philo *philo)
{
	long long	now;

	pthread_mutex_lock(&philo->state_mutex);
	now = now_us();
	if (is_dead_nolock(philo, now))
	{
		pthread_mutex_unlock(&philo->state_mutex);
		return (1);
	}
	philo->last_meal_us = now;
	pthread_mutex_unlock(&philo->state_mutex);
	return (0);
}

void	finish_eating(t_philo *philo)
{
	pthread_mutex_lock(&philo->state_mutex);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->state_mutex);
}

int	eat(t_philo *philo)
{
	if (begin_eating(philo))
		return (1);
	print_state(philo, "is eating");
	if (precise_sleep(philo->rules->time_to_eat_ms, philo->rules))
		return (1);
	return (0);
}
