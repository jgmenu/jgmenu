#include "../sbuf.h"
#include "../list.h"
#include "../util.h"

static char test_name[] = "test0002-sbuf";
static int verbose = 0;

int main(void)
{
	int i;
	struct sbuf s;

	sbuf_init(&s);
	for (i = 0; i < 5; i++) {
		sbuf_addch(&s, 'A' + i);
	}

	sbuf_addstr(&s, "FGHIJKLMNO");
	sbuf_addch(&s, 'P');
	sbuf_addch(&s, 'Q');
	sbuf_addstr(&s, "R");

	if (verbose) {
		printf("test:%s\n", s.buf);
		printf("length:%d\n", s.len);
		printf("allocated:%d\n", s.bufsiz);
	}

	if (!strcmp(s.buf, "ABCDEFGHIJKLMNOPQR") &&
	    s.len == 18 &&
	    s.bufsiz == 19) {
		printf("[PASS] %s\n", test_name);
	} else {
		printf("[FAIL] %s\n", test_name);
		return 1;
	}



	printf("\nString List Test:\n");

	struct list_head sl;	/* String List */
	struct sbuf *tmp;	/* temp string for the for_each loop */

	INIT_LIST_HEAD(&sl);
	sbuf_list_append(&sl, "zero");
	sbuf_list_append(&sl, "one");
	sbuf_list_append(&sl, "two");
	sbuf_list_append(&sl, "three");

	i = 0;
	list_for_each_entry(tmp, &sl, list)
		printf("list-item-%d: %s\n", i++, tmp->buf);

	printf("\n");

	/* Testing prepend */
	sbuf_cpy(&s, "bar");
	sbuf_prepend(&s, "foo");
	if (!strncmp(s.buf, "foobar", 6))
		printf("[PASS] - sbuf_prepend()\n\n");
	else
		exit(1);

	/* Testing sbuf_shift_left */
	sbuf_cpy(&s, "abcdefghijkl");
	sbuf_shift_left(&s, 3);
	printf("shift_left by 3: %s\n", s.buf);
	sbuf_cpy(&s, "abcdefghijkl");
	sbuf_shift_left(&s, 1);
	printf("shift_left by 1: %s\n", s.buf);
	printf("\n");

	/* Testing expand_tilde */
	sbuf_cpy(&s, "~/.config/jgmenu/jgmenurc");
	sbuf_expand_tilde(&s);
	printf("tilde expanded: %s\n", s.buf);
	printf("\n");

	/* Testing sbuf_split */
	sbuf_cpy(&s, "abc:def:ghi:jkl");
	struct list_head my_split_list;
	INIT_LIST_HEAD(&my_split_list);
	sbuf_split(&my_split_list, s.buf, ':');
	list_for_each_entry(tmp, &my_split_list, list)
		printf("SPLIT:%s\n", tmp->buf);

	printf("\n");
	printf("%d\n", get_first_num_from_str("axxx"));
	printf("%d\n", get_first_num_from_str("ax1xx"));
	printf("%d\n", get_first_num_from_str("22axxx"));
	printf("%d\n", get_first_num_from_str("axxx333"));
	printf("%d\n", get_first_num_from_str("a4444xxx"));

	sbuf_cpy(&s, "  xxxx  ");
	sbuf_rtrim(&s);
	printf("_%s_\n", s.buf);
	sbuf_cpy(&s, "  xxxx  ");
	sbuf_ltrim(&s);
	printf("_%s_\n", s.buf);
	sbuf_cpy(&s, "  xxxx  ");
	sbuf_trim(&s);
	printf("_%s_\n", s.buf);

	sbuf_cpy(&s, "Sound & Graphics");
	sbuf_replace(&s, "&", "&amp;");
	printf("_%s_\n", s.buf);

	sbuf_cpy(&s, "fooooooobksdfklsadjfafoo");
	printf("_%s_\n", s.buf);
	sbuf_replace(&s, "foo", "*****");
	printf("_%s_\n", s.buf);

	sbuf_cpy(&s, "foofoofoofoof");
	printf("_%s_\n", s.buf);
	sbuf_replace(&s, "oo", "");
	printf("_%s_\n", s.buf);

	sbuf_init(&s);
	printf("\nXXXXXXXXXXXXXXXXX\n\n");
	sbuf_cpy(&s, "one  two    three                         four");
	printf("_%s_\n", s.buf);
	sbuf_replace_spaces_with_one_tab(&s);
	printf("_%s_\n", s.buf);

	return 0;
}
