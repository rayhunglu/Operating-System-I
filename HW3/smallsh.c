#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#define LSH_TOK_BUFSIZE 512
#define LSH_TOK_DELIM " \t\r\n\a"

int smallsh_cd(char **args);
int smallsh_status(char **args);
int smallsh_exit(char **args);
char* INPUT;
char* OUTPUT;
char* tmpStr;
int shell_status;
int _isForeground=0;

void freeMalloc()
{
    if(INPUT != NULL)
        free(INPUT);
    if(OUTPUT != NULL)
        free(OUTPUT);
}

char *builtin_str[] = {
    "cd",
    "status",
    "exit"
};
int (*builtin_func[]) (char **) = {
    &smallsh_cd,
    &smallsh_status,
    &smallsh_exit
};
int smallsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}
/**
args[0] is "cd".  args[1] is the directory.
return Always returns 1, to continue executing.
*/
int smallsh_cd(char **args)
{
//    char *dir=args[1];
    char  *gdir;
    char  *dir;
    char  *to;
    char buf[1000];
    if (args[1] == NULL) {
        dir=getenv("HOME");
        chdir(dir);
    }else{
        gdir = getcwd(buf, sizeof(buf));
        dir = strcat(gdir, "/");
        to = strcat(dir, args[1]);
        chdir(to);
    }
    return 1;
}
//return Always returns 1, to continue executing.
int smallsh_status(char **args)
{
    //If the child exited, print its status
    if (WIFEXITED(shell_status))
        printf("exit value %d\n", WEXITSTATUS(shell_status));
    // If it was signalled, print the signal it received.
//    if (WIFSIGNALED(shell_status))
//        printf("stop signal %d\n", WSTOPSIG(shell_status));
    // If it was terminated, print the termination signal.
    if (WTERMSIG(shell_status))
        printf("terminated by signal %d\n",WTERMSIG(shell_status));
    
    return 1;
}
//return Always returns 0, to terminate execution.
int smallsh_exit(char **args)
{
    free(args);
    exit(0);
}
//execute stdin or stdout
void exeStdFile(char* fileName, FILE* io)
{
    int file = 0;
    if( io == stdout)
        file = open(fileName,O_WRONLY | O_CREAT, 0744);
    else
    {
        file = open(fileName, O_RDONLY);
        if( file == -1)
        {
            perror(fileName);
            exit(1);
        }
    }
    dup2(file,fileno(io));
    close(file);
}

//execute the command except cd, status, exit
int smallsh_launch(char **args,int* runningback)
{
    pid_t pid, wpid;
    
    pid = fork();
    if (pid == 0) {
        if(INPUT != NULL) exeStdFile(INPUT, stdin);
        if(OUTPUT != NULL) exeStdFile(OUTPUT, stdout);
        fflush(stdout);
        
        if (execvp(args[0], args) == -1) {
            perror(args[0]);
        }
        free(args);
        exit(1);
    }else if (pid < 0)
    {
        // Error forking
        perror("forking");
    }
    else
    {
        if(*runningback)
            printf("background pid is %d\n", pid);
        // Parent process
        else{
            do {
                wpid = waitpid(pid, &shell_status, WUNTRACED);
            } while (!WIFEXITED(shell_status) && !WIFSIGNALED(shell_status));
            
        }
    }
    return 1;
}

//return 1 if the shell should continue running, 0 if it should terminate
int smallsh_execute(char **args,int* runningback)
{
    int i;
    
    if (args[0] == NULL) {
        return 1;       // An empty command was entered.
    }
    for (i = 0; i < smallsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return smallsh_launch(args,runningback);
}

/**
 @brief Read a line of input from stdin.
 @return The line from stdin.
 */
char *smallsh_read_line(void)
{
    int bufsize = LSH_TOK_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    
    if (!buffer) {
        fprintf(stderr, "smallsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        // Read a character
        c = getchar();
        
        // If we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        
        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "smallsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char *replaceWord(const char *s, const char *oldW,const char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
    /*Loop string until the end*/
    for (i = 0; s[i] != '\0'; i++)
    {
        /*Count the how many place need to be changed*/
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;
            i += oldWlen - 1;
        }
    }
    
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);
    
    i = 0;
    /*Loop string until the end*/
    while (*s)
    {
        /*Replacing the new word*/
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
    
    result[i] = '\0';
    return result;
}
// bool checkand(char *token){
//     char* test=strstr(token,"&");
//     if(test[1]==NULL) return true;
//     return false;
// }
//Split a line into tokens and return
char **smallsh_split_line(char *line,int *runningback)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    INPUT = NULL;
    OUTPUT = NULL;
    if (!tokens) {
        fprintf(stderr, "smallsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        if(strcmp(token,"<")==0){
            token = strtok(NULL, LSH_TOK_DELIM);
            if(token == NULL) return tokens;
            
            INPUT = malloc(sizeof(char) * strlen(token));
            strcpy(INPUT, token);
            token = strtok(NULL, LSH_TOK_DELIM);
        }
        else if(strcmp(token,">")==0){
            token = strtok(NULL, LSH_TOK_DELIM);
            if(token == NULL) return tokens;
            
            OUTPUT = malloc(sizeof(char) * strlen(token));
            strcpy(OUTPUT, token);
            token = strtok(NULL, LSH_TOK_DELIM);
        }
        else if(strstr(token,"$$")!=0){
            pid_t p = getpid();
            sprintf(tmpStr, "%d", p);
            tokens[position] = replaceWord(token,"$$",tmpStr);
            position++;
            token = strtok(NULL, LSH_TOK_DELIM);
        }
        else if(strcmp(token,"&")==0 && strcmp(tokens[0], "echo") != 0){
            if(_isForeground == 0)
                *runningback = 1;
            token = strtok(NULL, LSH_TOK_DELIM);
        }
        else{
            tokens[position] = token;
            position++;
            token = strtok(NULL, LSH_TOK_DELIM);
        }
    };
    return tokens;
}
//find the defunct child process id
void traceDefunctPid(char **args)
{
    pid_t pid = waitpid(-1, &shell_status, WNOHANG);
    if(pid == -1 || pid == 0)
        return;
    
    printf("background pid %d is done: ", pid);
    int i=smallsh_status(args);
}
//executing loop
void smallsh_loop(void){
    char *line=NULL;
    char **args;
    int status;
    int runningback;
    tmpStr = malloc(sizeof(char) * 10);
    do {
        runningback=0;
        printf(":");
        line = smallsh_read_line();
        fflush(stdout);
        args = smallsh_split_line(line,&runningback);
        status = smallsh_execute(args,&runningback);
        
        traceDefunctPid(args);
        freeMalloc();
    } while (status);
}
//ctrl+Z
void sigstop(int p){
    signal(SIGTSTP,sigstop);
    if(_isForeground)
    {
        _isForeground = 0;
        printf("\nExiting foreground-only mode\n:");
    }
    else
    {
        _isForeground = 1;
        printf("\nEntering foreground-only mode (& is now ignored)\n:");
    }
    fflush(stdout);
}
// Ctrl+C
void sigkill(int p){
    int my_pid=waitpid(-1,&shell_status, WUNTRACED);
    if( my_pid == 0 || my_pid == -1) return;
    kill(my_pid,SIGKILL);
    if (WTERMSIG(shell_status))
        printf("terminated by signal: %d\n",WTERMSIG(p));
    fflush(stdout);
}
int main(int argc, char **argv)
{
    signal(SIGTSTP,sigstop);        // Ctrl+Z
    signal(SIGINT,sigkill);         // Ctrl+C
    
    smallsh_loop();                 // Run command loop.
    
    return EXIT_SUCCESS;
}
