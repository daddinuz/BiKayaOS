#include <system.h>
#include <printer.h>
#include <term.h>

int main(void) {
  int c;
	
	printer_puts("hello printer");

  while (1) {
    term_puts("you> ");

    for (c = term_getchar(); c >= 0 && c != '\n'; c = term_getchar()) {
      term_putchar(c);
    }

    term_putchar('\n');

  }

  return 0;
}
