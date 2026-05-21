*This project has been created as part of the 42 curriculum by stonegaw.*

# philosophers

## Description

`philosophers` is the 42 concurrency project about threads, mutexes, timing, and synchronization.

The program simulates philosophers sitting around a table. Each philosopher repeatedly:

- takes two forks
- eats
- sleeps
- thinks

The simulation must stop as soon as one philosopher dies, or when every philosopher has eaten enough times if the optional `number_of_times_each_philosopher_must_eat` argument is provided.

This implementation focuses on:

- correct thread-based synchronization with `pthread`
- race-free access to shared state
- accurate death detection
- understandable deadlock avoidance
- practical starvation mitigation for standard test cases

## Instructions

### Build

```bash
cd philo
make
```

### Run

```bash
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

Examples:

```bash
./philo 2 210 100 100      # should keep running without death
./philo 2 199 100 100      # should end with one death
./philo 3 630 200 200      # should keep running without death
./philo 5 800 200 200 7    # should exit after all philosophers eat 7 times
```

### Clean

```bash
make clean
make fclean
```

### Rebuild
```bash
make re
```

## Technical Choices

### Deadlock Avoidance

Each fork is represented by one mutex, and each philosopher stores pointers to its left and right fork.

Deadlock is avoided by enforcing a global lock order:

- each philosopher locks the lower fork id first
- then locks the higher fork id

This removes the circular-wait pattern where every philosopher grabs one fork and waits forever for the other.

### Death Detection

The main thread runs the monitor loop.

For each philosopher, the program tracks:

- `last_meal_us`
- `meals_eaten`

`last_meal_us` is updated at the start of eating, not at the end. This matches the subject requirement: a philosopher dies if they do not start eating within `time_to_die` milliseconds since the last meal start, or since the beginning of the simulation.

### Starvation Mitigation

This project does **not** claim a formal starvation-free guarantee. Instead, it applies practical scheduling adjustments that make the standard scenarios much more stable.

The implementation uses:

- an initial stagger for even-numbered philosophers
  - they wait for `time_to_eat / 2` before entering the main loop
- an extra delay after `thinking` when the philosopher count is odd
  - `max(0, 2 * time_to_eat - time_to_sleep)`

Why this helps:

- without staggering, many philosophers try to take their first fork at almost the same time
- with odd philosopher counts, the same contention pattern can repeat every cycle
- the extra think delay breaks that phase alignment and reduces the chance that the same philosopher keeps losing the same race

These choices are meant to reduce unfair recurring contention, not to mathematically prove fairness.

### Special Case: One Philosopher

The one-philosopher case is handled separately:

- the philosopher takes the only fork
- prints `has taken a fork`
- waits until the monitor reports death

They never print `is eating`, because two forks are required to eat.

## Resources

General references used for this project:

- POSIX Threads documentation
- `pthread_create(3)`, `pthread_join(3)`, `pthread_mutex_init(3)`, `pthread_mutex_lock(3)`, `gettimeofday(2)`, and `usleep(3)`
- [食事する哲学者の問題](https://ja.wikipedia.org/wiki/%E9%A3%9F%E4%BA%8B%E3%81%99%E3%82%8B%E5%93%B2%E5%AD%A6%E8%80%85%E3%81%AE%E5%95%8F%E9%A1%8C)

AI usage:

- AI assistance was used for design review, edge-case discussion, synchronization tradeoff analysis, and documentation drafting.
- Most of the implementation, and all review, final decisions, manual testing were done manually by the author.