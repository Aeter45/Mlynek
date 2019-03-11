#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include<string.h>

#include<gtk/gtk.h>
#include<gdk/gdk.h>
#include<glib.h>

#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>

typedef struct pipes *PipesPtr;

PipesPtr initPipes(int argc,char *argv[]);
void     sendStringToPipe(PipesPtr channel,const char *data);
bool     getStringFromPipe(PipesPtr channel, char *buffer, size_t size);
void     closePipes(PipesPtr channel);
