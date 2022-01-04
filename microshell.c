#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

typedef struct s_cmd t_cmd; 

struct s_cmd
{
	char	**args;
	t_cmd	*next;
};

void	ft_putstr(char *str, int fd)
{
	while (*str)
		write(fd, str++, 1);
}

int	is_sep(char *str)
{
	if (str[0] == '|' || str[0] == ';')
		return 1;
	else
		return 0;
}

int	get_cmds_nb(char **av, int ac, int i)
{
	int nb = 1;
	while (i < ac && av[i][0] != ';')
	{
		if (av[i][0] == '|')
			nb++;
		i++;
	}
	return nb;
}

int	get_args_nb(char **av, int ac, int i)
{
	int nb = 0;
	while (i < ac && !is_sep(av[i]))
	{
		i++;
		nb++;
	}
	return nb;
}
char	**get_args(char **av, int ac, int *i)
{
	char **args = malloc(sizeof(char *) * (get_args_nb(av, ac, *i) + 2));
	int j = 1;
	while (*i < ac && !is_sep(av[*i]))
	{
		args[j++] = av[*i];
		*i = *i + 1;
	}
	args[j] = NULL;
	return args;
}
t_cmd	*get_cmds(char **av, int ac, int *i)
{
	int cmd_nb = get_cmds_nb(av, ac, *i);
	t_cmd *cmds = malloc(sizeof(t_cmd) * cmd_nb);
	int j = 0;
	cmds[j].args = NULL;
	cmds[j].next = NULL;
	while (*i < ac && av[*i][0] != ';')
	{
		if (av[*i][0] != '|')
		{
			int save = *i;
			*i = *i + 1;
			cmds[j++].args = get_args(av, ac, i);
			cmds[j - 1].args[0] = av[save];
			if (j < cmd_nb)
				cmds[j - 1].next = &cmds[j];
			else
				cmds[j - 1].next = NULL;
			continue ;
		}
		*i = *i + 1;
	}
	return cmds;
}
void	child(t_cmd *cmd, int fd[2], char **env)
{
	if (cmd->next)
	{
		if (dup2(fd[1], 1) == -1)
		{
			ft_putstr("error: fatal\n", 2);
			exit(1);
		}
	}
	close(fd[0]);
	close(fd[1]);
	if (execve(cmd->args[0], cmd->args, env) < 0)
	{
		ft_putstr("error: cannot execute ", 2);
		ft_putstr(cmd->args[0], 2);
		ft_putstr("\n", 2);
	}
}
void	exec(t_cmd *cmds, char **env)
{
	while (cmds)
	{
		int fd[2];
		pipe(fd);
		pid_t pid = fork();
		if (pid == 0)
			child(cmds, fd, env);
		if (dup2(fd[0], 0) == -1)
		{
			ft_putstr("error: fatal\n", 2);
			exit(1);
		}
		wait(&pid);
		close(fd[0]);
		close(fd[1]);
		cmds = cmds->next;
	}
}

int main(int ac, char **av, char **env)
{
	int i = 0;
	while (++i < ac)
	{
		t_cmd *cmds = get_cmds(av, ac, &i);
		if (cmds->args)
			exec(cmds, env);
	}
	exit(0);
}
