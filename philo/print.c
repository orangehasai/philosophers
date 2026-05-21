/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:06 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 19:07:57 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	print_state(t_philo *philo, const char *msg)
{
	long long	ts;

	pthread_mutex_lock(&philo->rules->print_mutex);
	pthread_mutex_lock(&philo->rules->stop_mutex);
	if (philo->rules->stop != 1)
	{
		ts = elapsed_ms(philo->rules);
		printf("%lld %d %s\n", ts, philo->id, msg);
	}
	pthread_mutex_unlock(&philo->rules->stop_mutex);
	pthread_mutex_unlock(&philo->rules->print_mutex);
}

void	announce_death(t_philo *philo)
{
	long long	ts;

	pthread_mutex_lock(&philo->rules->print_mutex);
	pthread_mutex_lock(&philo->rules->stop_mutex);
	if (philo->rules->stop != 1)
	{
		philo->rules->stop = 1;
		ts = elapsed_ms(philo->rules);
		printf("%lld %d died\n", ts, philo->id);
	}
	pthread_mutex_unlock(&philo->rules->stop_mutex);
	pthread_mutex_unlock(&philo->rules->print_mutex);
}
