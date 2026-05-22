/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   action_sleep.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 22:11:21 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 22:50:34 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	philo_sleep(t_philo *philo)
{
	if (!can_take_next_action(philo))
		return (1);
	print_state(philo, "is sleeping");
	return (precise_sleep(philo->rules->time_to_sleep_ms, philo->rules));
}
