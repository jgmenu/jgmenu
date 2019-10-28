#ifndef BANNED_H
#define BANNED_H

/*
 * Inspired by
 *   - http://github.com/git/git/blob/master/banned.h
 *   - http://github.com/leafsr/gcc-poison/blob/master/poison.h
 *
 * Force compile-time error on use of functions which are considered banned
 * because they are easy to misuse and complicate audits.
 */

#define BANNED(func) sorry_##func##_is_a_banned_function

#undef strcpy
#define strcpy(...) BANNED(strcpy)

#undef strcat
#define strcat(...) BANNED(strcat)

#undef strncpy
#define strncpy(...) BANNED(strncpy)

#undef strncat
#define strncat(...) BANNED(strncat)

#undef sprintf
#define sprintf(...) BANNED(sprintf)

#undef vsprintf
#define vsprintf(...) BANNED(vsprintf)

#undef wcscpy
#define wcscpy(...) BANNED(wcscpy)

#undef stpcpy
#define stpcpy(...) BANNED(stpcpy)

#undef wcpcpy
#define wcpcpy(...) BANNED(wcpcpy)

#undef scanf
#define scanf(...) BANNED(scanf)

#undef sscanf
#define sscanf(...) BANNED(sscanf)

#undef vscanf
#define vscanf(...) BANNED(vscanf)

#undef fwscanf
#define fwscanf(...) BANNED(fwscanf)

#undef swscanf
#define swscanf(...) BANNED(swscanf)

#undef wscanf
#define wscanf(...) BANNED(wscanf)

#undef gets
#define gets(...) BANNED(gets)

#undef puts
#define puts(...) BANNED(puts)

#undef wcscat
#define wcscat(...) BANNED(wcscat)

#undef wcrtomb
#define wcrtomb(...) BANNED(wcrtomb)

#undef wctob
#define wctob(...) BANNED(wctob)

#undef asprintf
#define asprintf(...) BANNED(asprintf)

#undef vasprintf
#define vasprintf(...) BANNED(vasprintf)

#undef wcsncpy
#define wcsncpy(...) BANNED(wcsncpy)

#undef strtok
#define strtok(...) BANNED(strtok)

#undef wcstok
#define wcstok(...) BANNED(wcstok)

#undef strdupa
#define strdupa(...) BANNED(strdupa)

#undef strndupa
#define strndupa(...) BANNED(strndupa)

#undef longjmp
#define longjmp(...) BANNED(longjmp)

#undef siglongjmp
#define siglongjmp(...) BANNED(siglongjmp)

#undef setjmp
#define setjmp(...) BANNED(setjmp)

#undef sigsetjmp
#define sigsetjmp(...) BANNED(sigsetjmp)

#undef mallopt
#define mallopt(...) BANNED(mallopt)

#undef remove
#define remove(...) BANNED(remove)

#undef mktemp
#define mktemp(...) BANNED(mktemp)

#undef tmpnam
#define tmpnam(...) BANNED(tmpnam)

#undef tempnam
#define tempnam(...) BANNED(tempnam)

#undef cuserid
#define cuserid(...) BANNED(cuserid)

#undef rexec
#define rexec(...) BANNED(rexec)

#undef rexec_af
#define rexec_af(...) BANNED(rexec_af)

#endif /* BANNED_H */
