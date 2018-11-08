#include <stdio.h>
#include <stdlib.h>

/* usage: filter-out [<line_numbers...>] */

int main(int argc, char **argv)
{
	int i, j;
	char line[BUFSIZ];

	for (i = 0; fgets(line, sizeof(line), stdin); i++) {
		for (j = 0; j < argc; j++)
			if (atoi(argv[j]) == i + 1)
				goto out;
		printf("%s", line);
out:
		;
	}

	return 0;
}
