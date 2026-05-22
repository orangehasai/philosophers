/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   action_fork.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:55:52 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 22:11:30 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static pthread_mutex_t	*first_fork(t_philo *philo)
{
	if (philo->left_fork_id < philo->right_fork_id)
		return (philo->left_fork);
	return (philo->right_fork);
}

static pthread_mutex_t	*second_fork(t_philo *philo)
{
	if (philo->left_fork_id < philo->right_fork_id)
		return (philo->right_fork);
	return (philo->left_fork);
}

int	take_forks(t_philo *philo)
{
	pthread_mutex_lock(first_fork(philo));
	if (!can_take_next_action(philo))
	{
		pthread_mutex_unlock(first_fork(philo));
		return (1);
	}
	print_state(philo, "has taken a fork");
	pthread_mutex_lock(second_fork(philo));
	if (!can_take_next_action(philo))
	{
		pthread_mutex_unlock(second_fork(philo));
		pthread_mutex_unlock(first_fork(philo));
		return (1);
	}
	print_state(philo, "has taken a fork");
	return (0);
}

void	put_forks(t_philo *philo)
{
	pthread_mutex_unlock(second_fork(philo));
	pthread_mutex_unlock(first_fork(philo));
}
