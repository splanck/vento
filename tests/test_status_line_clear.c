#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ncurses.h>
#undef mvprintw
#undef clrtoeol
#undef refresh

static char line[81];
static char displayed[81];
static int cursor_end;

int mvprintw(int y,int x,const char*fmt,...){(void)y;va_list ap;va_start(ap,fmt);char buf[81];int n=vsnprintf(buf,sizeof(buf),fmt,ap);va_end(ap);if(n<0)n=0;if(x+n>80)n=80-x;memcpy(line+x,buf,n);cursor_end=x+n;return 0;}
int clrtoeol(void){for(int i=cursor_end;i<80;++i)line[i]=' ';line[80]='\0';return 0;}
int refresh(void){memcpy(displayed,line,sizeof(line));return 0;}

static void rtrim(char*s){for(int i=79;i>=0&&s[i]==' ';--i)s[i]='\0';}

int main(void){
    memset(line,' ',sizeof(line));line[80]='\0';
    mvprintw(22,0,"First long message");
    clrtoeol();
    refresh();
    mvprintw(22,0,"Short");
    clrtoeol();
    refresh();
    char buf[81];memcpy(buf,displayed,sizeof(buf));rtrim(buf);assert(strcmp(buf,"Short")==0);
    return 0;
}
