/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   action_think.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 22:11:19 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 22:47:40 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	think(t_philo *philo)
{
	long long	think_ms;

	if (!can_take_next_action(philo))
		return (1);
	print_state(philo, "is thinking");
	if (philo->rules->num_philo % 2 == 0)
		return (0);
	think_ms = philo->rules->time_to_eat_ms * 2
		- philo->rules->time_to_sleep_ms;
	if (think_ms > 0)
		return (precise_sleep(think_ms, philo->rules));
	return (0);
}
