/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:32 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/21 15:46:27 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <limits.h>
# include <pthread.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_rules
{
	int				num_philo;
	long long		time_to_die_ms;
	long long		time_to_eat_ms;
	long long		time_to_sleep_ms;
	int				must_eat_count;
	long long		start_us;
	int				stop;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	*forks;
	struct s_philo	*philos;
}					t_rules;

typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long long		last_meal_us;
	pthread_t		thread;
	pthread_mutex_t	state_mutex;
	int				left_fork_id;
	int				right_fork_id;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	t_rules			*rules;
}					t_philo;

int					parse_args(int argc, char **argv, t_rules *rules);
long long			now_us(void);
long long			elapsed_ms(t_rules *rules);
int					simulation_should_stop(t_rules *rules);
void				set_stop_flag(t_rules *rules, int value);
int					init_data(t_rules *rules);
void				cleanup_arrays(t_rules *rules);
void				cleanup_init_failure(t_rules *rules, int fork_count,
						int state_count);
void				cleanup_all(t_rules *rules);
#endif
