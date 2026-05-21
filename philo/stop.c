/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 14:04:27 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 14:09:55 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

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
