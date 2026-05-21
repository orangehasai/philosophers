/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:03 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/22 00:07:04 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	parse_error(void)
{
	write(2, "Error\n", 6);
	return (1);
}

static int	parse_positive_ll(char *str, long long *value)
{
	int			i;
	int			digit;
	long long	result;

	i = 0;
	result = 0;
	if (str[i] == '+')
		i++;
	if (str[i] == '\0')
		return (1);
	while (str[i] != '\0')
	{
		if (str[i] < '0' || str[i] > '9')
			return (1);
		digit = str[i] - '0';
		if (result > (LLONG_MAX - digit) / 10)
			return (1);
		result = result * 10 + digit;
		i++;
	}
	if (result == 0)
		return (1);
	*value = result;
	return (0);
}

static int	parse_int_arg(char *str, int *value)
{
	long long	parsed;

	if (parse_positive_ll(str, &parsed))
		return (1);
	if (parsed > INT_MAX)
		return (1);
	*value = (int)parsed;
	return (0);
}

static int	parse_time_arg(char *str, long long *value)
{
	if (parse_positive_ll(str, value))
		return (1);
	if (*value > LLONG_MAX / 1000LL)
		return (1);
	return (0);
}

int	parse_args(int ac, char **av, t_rules *rules)
{
	if (ac != 5 && ac != 6)
		return (parse_error());
	if (parse_int_arg(av[1], &rules->num_philo) || parse_time_arg(av[2],
			&rules->time_to_die_ms) || parse_time_arg(av[3],
			&rules->time_to_eat_ms) || parse_time_arg(av[4],
			&rules->time_to_sleep_ms))
		return (parse_error());
	rules->must_eat_count = MUST_EAT_UNSET;
	if (ac == 6 && parse_int_arg(av[5], &rules->must_eat_count))
		return (parse_error());
	return (0);
}
