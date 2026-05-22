/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stonegaw <stonegaw@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 22:56:32 by stonegaw          #+#    #+#             */
/*   Updated: 2026/05/22 00:10:34 by stonegaw         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# define MUST_EAT_UNSET -1

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
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
int					is_dead_nolock(t_philo *philo, long long now_us);
int					is_dead(t_philo *philo);
int					can_take_next_action(t_philo *philo);
int					simulation_should_stop(t_rules *rules);
void				set_stop_flag(t_rules *rules, int value);
int					init_data(t_rules *rules);
void				cleanup_arrays(t_rules *rules);
void				cleanup_init_failure(t_rules *rules, int fork_count,
						int state_count);
int					cleanup_all(t_rules *rules);
void				print_state(t_philo *philo, const char *msg);
void				announce_death(t_philo *philo);
int					take_forks(t_philo *philo);
void				put_forks(t_philo *philo);
void				handle_one_philo(t_philo *philo);
int					start_routine(t_rules *rules, int *started_count);
int					join_threads(t_rules *rules, int started_count);
int					eat(t_philo *philo);
int					precise_sleep(long long duration_ms, t_rules *rules);
int					philo_sleep(t_philo *philo);
int					think(t_philo *philo);
void				monitor(t_rules *rules);

#endif
