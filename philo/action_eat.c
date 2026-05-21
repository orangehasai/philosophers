/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   action_eat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 22:11:25 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 23:06:04 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	begin_eating(t_philo *philo)
{
	pthread_mutex_lock(&philo->state_mutex);
	philo->last_meal_us = now_us();
	pthread_mutex_unlock(&philo->state_mutex);
}

static void	finish_eating(t_philo *philo)
{
	pthread_mutex_lock(&philo->state_mutex);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->state_mutex);
}

void	eat(t_philo *philo)
{
	begin_eating(philo);
	print_state(philo, "is eating");
	if (!precise_sleep(philo->rules->time_to_eat_ms, philo->rules))
		finish_eating(philo);
}
