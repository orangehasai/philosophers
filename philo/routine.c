/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:04 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 20:16:06 by stonegaw         ###   ########.fr       */
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
