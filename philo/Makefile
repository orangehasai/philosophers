NAME = philo

CC = cc

CFLAGS = -Wall -Wextra -Werror -pthread

SRCS = actions.c \
		cleanup.c \
		init.c \
		main.c \
		monitor.c \
		parse.c \
		print.c \
		routine.c \
		time.c \

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c philo.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
