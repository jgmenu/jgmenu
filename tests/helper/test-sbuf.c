#include "sbuf.h"
#include "list.h"
#include "util.h"

#define DELIM " \t\r\n"
int main(int argc, char **argv)
{
	struct sbuf s;
        char line[1024];

	sbuf_init(&s);

        while (fgets(line, sizeof(line), stdin)) {
                char *cmd, *p1 = NULL;

                cmd = strtok(line, DELIM);
                if (!cmd || *cmd == '#')
                        continue;
                p1 = strtok(NULL, DELIM);

                if (!strcmp("addch", cmd) && p1)
                        sbuf_addch(&s, p1[0]);
                else if (!strcmp("addstr", cmd) && p1)
                        sbuf_addstr(&s, p1);
                else if (!strcmp("cpy", cmd) && p1)
                        sbuf_cpy(&s, p1);
                else if (!strcmp("prepend", cmd) && p1)
                        sbuf_prepend(&s, p1);
                else if (!strcmp("shiftleft", cmd) && p1)
                        sbuf_shift_left(&s, atoi(p1));
                else if (!strcmp("len", cmd))
                        printf("%d\n", s.len);
                else if (!strcmp("bufsiz", cmd))
                        printf("%d\n", s.bufsiz);
                else if (!strcmp("print", cmd))
                        printf("%s\n", s.buf);
        }

        xfree(s.buf);
        return 0;
}
