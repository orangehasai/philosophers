/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:08 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 23:07:52 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

long long	now_us(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000LL + tv.tv_usec);
}

long long	elapsed_ms(t_rules *rules)
{
	return ((now_us() - rules->start_us) / 1000LL);
}

int	precise_sleep(long long duration_ms, t_rules *rules)
{
	long long	start_us;
	long long	target_us;

	start_us = now_us();
	target_us = duration_ms * 1000LL;
	while (!simulation_should_stop(rules))
	{
		if (now_us() - start_us >= target_us)
			return (0);
		usleep(100);
	}
	return (1);
}
