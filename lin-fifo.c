#include"mlynek.h"

struct pipes {
    FILE *fifo_in, *fifo_out;
    int isA;
};

int fileno(FILE* file);
static FILE *openOutPipe(char *name);
static FILE *openInPipe(char *name);

void closePipes(PipesPtr pipes)
{
    fclose(pipes->fifo_in);
    fclose(pipes->fifo_out);
    free(pipes);    
}

PipesPtr initPipes(int argc,char *argv[])
{
    if(argc < 2 || (argv[1][0] != 'A' && argv[1][0] != 'B') || argv[1][1] != '\0') 
    {
        fprintf(stderr, "\nThis program should be called with the first argument: A or B\n\n");
        mkfifo("AtoB", 0644);
        mkfifo("BtoA", 0644);
        fprintf(stderr, "Fifo queues AtoB and BtoA created\n");
        return NULL;
    }    
    PipesPtr pipes = (PipesPtr)malloc(sizeof(struct pipes));
    if (pipes == NULL) {
        fprintf(stderr, "Memory allocation error\n");
    } else {
        pipes->isA = (argv[1][0] == 'A');
        pipes->fifo_out = openOutPipe(pipes->isA ? "AtoB" : "BtoA");
        pipes->fifo_in  = openInPipe(pipes->isA  ? "BtoA" : "AtoB");
    }
    return pipes;
}

static FILE *openOutPipe(char *name) 
{
    mkfifo(name, 0644);
    FILE *pipe = fopen(name, "w+");
    if(pipe == NULL) {
        showError("Error in creating output pipe");
        exit(-1);
    }
    return pipe;
}

static FILE *openInPipe(char *name) 
{
    FILE *pipe = fopen(name, "r+");
    if(pipe == NULL) {
        mkfifo(name, 0644);
        pipe = fopen(name, "r+");
    }
    if(pipe == NULL) {
        showError("Error in creating input pipe");
        exit(-1);
    }
    int flags, fd;
    fd = fileno(pipe);
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return pipe;
}

void sendStringToPipe(PipesPtr pipes, const char *data) 
{

    int result = fprintf(pipes->fifo_out, "%s", data);
    fflush(pipes->fifo_out);
    if(result == 0) 
        showError("Failed to send data");
}

bool getStringFromPipe(PipesPtr pipes, char *buffer, size_t size) 
{

    char *result = fgets(buffer, size, pipes->fifo_in);
    fflush(pipes->fifo_in);
    //if(result == NULL) showError("Failed to read data");
    return result != NULL;
}
