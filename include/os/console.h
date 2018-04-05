
#ifndef _OS_CONSOLE_H_
#define _OS_CONSOLE_H_

void initconsole(_Device *dev);
void setcursor(unsigned int x, unsigned int y, char on);
unsigned int getcursorx();
unsigned int getcursory();
char getcursoron();
void printcharpos(char ch, int x, int y);
void printchar(char ch);
void printstring(char *buf);

#endif
