#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/wait.h> 

#define INTERACTIVE_MODE 1
#define BATCH_MODE 2
#define FALSE 0
#define TRUE 1
#define BUFF_SIZE 64
char* curPathDir;
char* dirs[] = {"/usr/local/sbin", "/usr/local/bin", "/usr/sbin", "/usr/bin", "/sbin", "/bin"};

char *strdup(const char *s) {
   size_t len = strlen(s) + 1;
   void *new = malloc(len);
   if (new == NULL) return NULL;
   return (char *) memcpy(new, s, len);
}

FILE *fdopen(int fd, const char *mode) {
   int flags, perms;
   
   if (mode[0] == 'r') {
      flags = O_RDONLY;
      perms = 0;
   } else if (mode[0] == 'w') {
      flags = O_WRONLY | O_CREAT | O_TRUNC;
      perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
   } else if (mode[0] == 'a') {
      flags = O_WRONLY | O_CREAT | O_APPEND;
      perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
   } else {
      return NULL;
   }
   
   int newfd = dup(fd);
   if (newfd == -1) {
      return NULL;
   }
   
   FILE *fp = fdopen(newfd, mode);
   if (fp == NULL) {
      close(newfd);
      return NULL;
   }
   
   return fp;
}

int pwd(){
    printf("%s\n", curPathDir);
    return EXIT_SUCCESS;
}

int cd(char *path){
    char* curCd;
    curCd = strtok(path, " ");
    if(curCd != NULL){
        char* curCommand;
        int token_len = 0;
        char command_copy[strlen(curCd) + 1];
        strcpy(command_copy, curCd);
        curCommand = strtok(command_copy, "/");
        while(curCommand != NULL){
            curCommand = strtok(NULL, "/");
            token_len += 1;
        }
        int token_index = 0;
        char* commandList[token_len];
        char command_copy2[strlen(curCd) + 1];
        strcpy(command_copy2, curCd);
        curCommand = strtok(command_copy2, "/");
        while (curCommand != NULL) {
            commandList[token_index] = curCommand;
            curCommand = strtok(NULL, "/");
            token_index++;
        }
        char curPathDirCopy[1024];
        strcpy(curPathDirCopy, curPathDir);
        for(int i = 0; i < token_len; i++){
            //printf("%s, ",commandList[i]);
            if(strcmp(commandList[i], "..") == 0){
                for(int j=strlen(curPathDirCopy); j >= 0; j--){
                    if(curPathDirCopy[j]=='/'){
                        curPathDirCopy[j] = '\0';
                        break;
                    }
                }
            }
            else if(strcmp(commandList[i], "~") == 0){
                strcpy(curPathDirCopy, getenv("HOME"));
            }
            else{
                int totalSize = strlen(curPathDirCopy) + 1 + strlen(commandList[i]);
                strcat(curPathDirCopy, "/");
                strcat(curPathDirCopy, commandList[i]);
                curPathDirCopy[totalSize] = '\0';
            }
        }
        struct stat buf;
        // printf("\n");
        // printf("copy: %s\n", curPathDirCopy);
        int status = stat(curPathDirCopy, &buf);
        // update directory
        if(status == 0){
            curPathDir[0] = '\0';
            strcpy(curPathDir, curPathDirCopy);
            chdir(curPathDir);
            return EXIT_SUCCESS;
        }
        else{
            return EXIT_FAILURE;
        }
    }
    else{   
        curPathDir[0] = '\0';
        strcpy(curPathDir, getenv("HOME"));
        chdir(curPathDir);
    }
    return EXIT_SUCCESS;
}

int expand_wildcards(char ***commandListPtr, int size) {
    char **commandList = *commandListPtr;
    glob_t glob_results;
    int glob_flags = 0;
    int new_argc = 0;
    char **new_argv = malloc((size+5) * sizeof(char *));

    for (int i = 0; i < size-1; ++i) {
        if (strchr(commandList[i], '*')) {
            glob(commandList[i], glob_flags, NULL, &glob_results);
            if(glob_results.gl_pathc == 0){
                new_argv[new_argc++] = strdup(commandList[i]);
            }
            else{
                new_argv = realloc(new_argv, (new_argc + glob_results.gl_pathc + 6) * sizeof(char *));
                for (size_t j = 0; j < glob_results.gl_pathc; ++j) {
                    new_argv[new_argc++] = strdup(glob_results.gl_pathv[j]);
                }
            }
            globfree(&glob_results);
        } else {
            new_argv[new_argc++] = strdup(commandList[i]);
        }
    }

    new_argv[new_argc] = NULL;
    //printf("2: %d\n",size);
    // Replace the original command list with the expanded one.
    for (int i = 0; i < size; ++i) {
        free(commandList[i]);
    }
    free(commandList);

    *commandListPtr = new_argv;

    return new_argc + 1;
}

char** getTokens(char* command){
    //printf("command: %s\n", command);
    int numTokens = 0;
    int newSequence = TRUE;
    int tmp = TRUE;
    for(int i = 0; i < strlen(command); i++){
        if(command[i] == '>'){
            newSequence = TRUE;
        }
        else if(command[i] == '<'){
            newSequence = TRUE;
        }
        else if(command[i] == '|'){
            newSequence = TRUE;
        }
        else if(tmp==TRUE && (command[i] == ' ') && ((i+1) < strlen(command)) && (command[i+1] != ' ' && command[i+1] != '<' && command[i+1] != '>' && command[i+1] != '|')){
            numTokens++;
            newSequence = FALSE;
            continue;
        }
        else if(newSequence == TRUE){
            tmp = TRUE;
            if(i==0 && command[i]==' '){
                newSequence = FALSE;
                continue;
            }
            numTokens++;
            newSequence = FALSE;
        }
        else if(((i-1) >= 0) && (command[i-1] == '>' || command[i-1] == '<' || command[i-1] == '|')){
            if(command[i]==' '){
                tmp = FALSE;
            }
            else{
                newSequence = TRUE;
            }
        }
        if(newSequence == TRUE){
            tmp = TRUE;
            if(i==0 && command[i]==' '){
                newSequence = FALSE;
                continue;
            }
            numTokens++;
            newSequence = FALSE;
        }
    }
    int indices[numTokens];
    int curIndex = 0;
    tmp = TRUE;
    newSequence = TRUE;
    for(int i = 0; i < strlen(command); i++){
        if(command[i] == '>'){
            newSequence = TRUE;
        }
        else if(command[i] == '<'){
            newSequence = TRUE;
        }
        else if(command[i] == '|'){
            newSequence = TRUE;
        }
        else if(tmp==TRUE && (command[i] == ' ') && ((i+1) < strlen(command)) && (command[i+1] != ' ' && command[i+1] != '<' && command[i+1] != '>' && command[i+1] != '|')){
            indices[curIndex] = i+1;
            //printf("(%d)\n",i+1);
            curIndex++;
            newSequence = FALSE;
            continue;
        }
        else if(newSequence == TRUE){
            tmp = TRUE;
            if(i==0 && command[i]==' '){
                newSequence = FALSE;
                continue;
            }
            indices[curIndex] = i;
            //printf("(%d)\n",i);
            curIndex++;
            newSequence = FALSE;
        }
        else if(((i-1) >= 0) && (command[i-1] == '>' || command[i-1] == '<' || command[i-1] == '|')){
            if(command[i]==' '){
                tmp = FALSE;
            }
            else{
                newSequence = TRUE;
            }
        }
        if(newSequence == TRUE){
            tmp = TRUE;
            if(i==0 && command[i]==' '){
                newSequence = FALSE;
                continue;
            }
            indices[curIndex] = i;
            //printf("(%d)\n",i);
            curIndex++;
            newSequence = FALSE;
        }
    }
    char** parse1 = malloc((numTokens+2) * sizeof(char*));
    int start_index;
    int end_index;
    int i;
    char numTokensStr[1024];
    sprintf(numTokensStr, "%d", numTokens);
    parse1[0] = numTokensStr;
    for(i=0; i < numTokens-1; i++){
        start_index = indices[i];
        end_index = indices[i+1];
        char* substr = malloc(end_index - start_index + 1);
        strncpy(substr, command + start_index, end_index - start_index);
        substr[end_index - start_index] = '\0';
        parse1[i+1] = substr;
    }
    start_index = indices[i];
    end_index = strlen(command);
    char* substr = malloc(end_index - start_index + 1);
    strncpy(substr, command + start_index, end_index - start_index);
    substr[end_index - start_index] = '\0';
    parse1[numTokens] = substr;
    parse1[numTokens+1] = NULL;
    for(int j=0;j<numTokens+1;j++){
        for(int k=0;k<strlen(parse1[j]);k++){
            if(parse1[j][k] == ' '){
                parse1[j][k] = '\0';
                break;
            }
        }
        //printf("[%s] ", parse1[j]);
    }
    //printf("\n");
    return parse1;
}

void checkDirs(int commandListSize, char** commandList){
    
    char tmpCommand[strlen(curPathDir) + 1 + strlen(commandList[0]) + 1];
    strcpy(tmpCommand, curPathDir);
    strcat(tmpCommand, "/");
    strcat(tmpCommand, commandList[0]);
    // printf("full command: %s\n", full_command);
    struct stat buf;
    // printf("testing command 1: %s\n", tmpCommand);
    int status = stat(tmpCommand, &buf);
    //run curPathDir
    if(status != 0){
        // printf("FIRST COMMAND DIDNT WORK\n");
        int foundCommand = FALSE;
        //try looking through local directories for command
        int i;
        char* newCommand = malloc(strlen(dirs[0]) + 1 + strlen(commandList[0]) + 1);
        for (i = 0; i < 6; i++){
            strcpy(newCommand, dirs[i]);
            strcat(newCommand, "/");
            strcat(newCommand, commandList[0]);
            newCommand[strlen(dirs[0]) + 1 + strlen(commandList[0])] = '\0';
            // char err1[1024];
            // sprintf(err1, "Executing command: %s + %s", newCommand, commandList[0]);
            // perror(err1);
            //printf("testing command 2: %s\n", newCommand);
            int new_status = stat(newCommand, &buf);
            if(new_status == 0){
                if(access(newCommand, X_OK) == 0){
                    foundCommand = TRUE;
                    //printf("FOUND IT !\n");
                }
                break;
            }
            else{
                newCommand[0] = '\0';
            }
        }
        if(foundCommand == TRUE){
            //printf("Error executing command1: %s + %s + size: %d\n", newCommand, commandList[0], commandListSize);
            char err[1024];
            sprintf(err, "Error executing command1: %s + %s", newCommand, commandList[0]);
            execv(newCommand, commandList);
            perror(err);
            free(newCommand);
            exit(1);
        }
        else{
            free(newCommand);
            exit(1);
        }
    }
    else{
        char err[1024];
        sprintf(err, "Error executing command2: %s + %s + (%d)", tmpCommand, commandList[0], commandListSize);
        execv(tmpCommand, commandList);
        perror(err);
        exit(1);     
    }
}

int checkBuiltIn(char** commandList){
    return strcmp(commandList[0],"cd") == 0 || strcmp(commandList[0],"pwd")==0 || strcmp(commandList[0],"exit")==0;
}

int runBuiltIn(char** commandList, int commandListSize, int out_fd){
    int command_failed = FALSE;
    if(commandListSize >= 2){
        if(strcmp(commandList[0], "cd") == 0) {
            if(commandListSize<=3){
                command_failed = cd(commandList[1]);
            }
            else{
                command_failed = TRUE;
            }
        }
        else if(strcmp(commandList[0], "pwd") == 0){
            for (int i = 1; i < commandListSize; i++) {
                if (commandList[i] && strcmp(commandList[i], ">") == 0) {
                    commandList[i] = NULL;  // Terminate the command list before the redirection token
                    if (commandList[i + 1]) {
                        out_fd = open(commandList[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                        if (out_fd < 0) {
                            command_failed = TRUE;
                            break;
                        }
                    } else {
                        command_failed = TRUE;
                        break;
                    }
                }
            }
            if (!command_failed) {
                if (out_fd != -1) {
                    int stdout_bak = dup(STDOUT_FILENO);
                    dup2(out_fd, STDOUT_FILENO);
                    command_failed = pwd();
                    dup2(stdout_bak, STDOUT_FILENO);
                    close(out_fd);
                } else {
                    command_failed = pwd();
                }
            }
        }
        else if (strcmp(commandList[0], "exit") == 0) {
            for (int i = 1; i < commandListSize; i++) {
                if (commandList[i] && strcmp(commandList[i], ">") == 0) {
                    commandList[i] = NULL;  // Terminate the command list before the redirection token
                    if (commandList[i + 1]) {
                        out_fd = open(commandList[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                        if (out_fd < 0) {
                            command_failed = TRUE;
                            break;
                        }
                    } 
                    else{
                        command_failed = TRUE;
                        break;
                    }
                }
            }
            if (!command_failed) {
                if (out_fd != -1) {
                    int stdout_bak = dup(STDOUT_FILENO);
                    dup2(out_fd, STDOUT_FILENO);
                    printf("exiting...\n");
                    dup2(stdout_bak, STDOUT_FILENO);
                    close(out_fd);
                } else {
                    printf("exiting...\n");
                }
                for(int i=0;i<commandListSize-1;i++){
                    free(commandList[i]);
                }
                free(commandList);
                exit(0);
            } else {
                command_failed = TRUE;
            }
        }
    }
    else{
        command_failed = TRUE;
    }
    return command_failed;
}

void redirection(char** commandList, int commandListSize){
    int in_fd = -1;
    int out_fd = -1;

    for (int i = 0; i < commandListSize; i++) {
        if (commandList[i] != NULL && strcmp(commandList[i], "<") == 0) {
            if (commandList[i + 1] != NULL) {
                in_fd = open(commandList[i + 1], O_RDONLY);
                if (in_fd < 0) {
                    perror("Error opening input file");
                    exit(1);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
                commandList[i] = NULL;
                commandList[i + 1] = NULL;
            } else {
                fprintf(stderr, "Error: No input file specified\n");
                exit(1);
            }
        } else if (commandList[i] != NULL && strcmp(commandList[i], ">") == 0) {
            if (commandList[i + 1] != NULL) {
                out_fd = open(commandList[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                if (out_fd < 0) {
                    perror("Error opening output file");
                    exit(1);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
                commandList[i] = NULL;
                commandList[i + 1] = NULL;
            } else {
                fprintf(stderr, "Error: No output file specified\n");
                exit(1);
            }
        }
    }
}

int my_pipe(char* command1[], int size1, char* command2[], int size2) {
    int pipe_fd[2];
    pid_t pid1 = -1, pid2 = -1;
    int commandFailed1 = FALSE;
    int commandFailed2 = FALSE;
    int status1, status2;

    if (pipe(pipe_fd) < 0) {
        perror("Error creating pipe");
        exit(1);
    }

    if (checkBuiltIn(command1) == TRUE) {
        commandFailed1 = runBuiltIn(command1, size1, pipe_fd[1]);
    } else {
        pid1 = fork();
        if (pid1 < 0) {
            perror("Error forking first child process");
            exit(1);
        } else if (pid1 == 0) {
            // Child process 1
            dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to pipe write end
            close(pipe_fd[0]); // Close pipe read end
            redirection(command1, size1);
            checkDirs(size1, command1);
        }
    }

    if (pid1 > 0) {
        waitpid(pid1, &status1, 0); // Wait for child process 1 to complete
    }

    if (checkBuiltIn(command2) == TRUE) {
        commandFailed2 = runBuiltIn(command2, size2, pipe_fd[0]);
    } else {
        pid2 = fork();
        if (pid2 < 0) {
            perror("Error forking second child process");
            exit(1);
        } else if (pid2 == 0) {
            // Child process 2
            dup2(pipe_fd[0], STDIN_FILENO); // Redirect stdin to pipe read end
            close(pipe_fd[1]); // Close pipe write end
            redirection(command2, size2);
            checkDirs(size2, command2);
        }
    }

    if (pid2 > 0) {
        waitpid(pid2, &status2, 0); // Wait for child process 2 to complete
    }

    // Parent process
    close(pipe_fd[0]); // Close pipe read end
    close(pipe_fd[1]); // Close pipe write end

    if (pid1 == -1) {
        status1 = commandFailed1 ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    if (pid2 == -1) {
        status2 = commandFailed2 ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    if ((WEXITSTATUS(status1) || WEXITSTATUS(status2)) == TRUE) {
        perror("ERROR");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int tryExecute(char* path, char* full_command){

    int command_failed = FALSE;
    char command_tmp[strlen(full_command) + 1];
    strcpy(command_tmp, full_command);
    command_tmp[strlen(full_command)] = '\0';
    char* tok1 = strtok(command_tmp, "|");
    char* tok2 = strtok(NULL, "|");
    //if there is a pipe
    if(tok2 != NULL){
        char** tmpToken1 = getTokens(tok1);
        int token_len1 = atoi(tmpToken1[0]) + 1;
        if(token_len1 <= 1){
            free(tmpToken1);
            perror("PIPE ERROR");
            return EXIT_FAILURE;
        }
        char **commandList1 = malloc((token_len1) * sizeof(char*));
        for(int k=1;k<token_len1+1;k++){
            commandList1[k-1] = tmpToken1[k];
            //printf("{%s} - ", commandList1[k-1]);
        }
        token_len1 = expand_wildcards(&commandList1, token_len1);
        //printf("\n");
        char** tmpToken2 = getTokens(tok2);
        int token_len2 = atoi(tmpToken2[0]) + 1;
        if(token_len2 <= 1){
            free(tmpToken2);
            perror("PIPE ERROR");
            return EXIT_FAILURE;
        }
        char **commandList2 = malloc((token_len2) * sizeof(char*));
        for(int k=1;k<token_len2+1;k++){
            commandList2[k-1] = tmpToken2[k];
            //printf("{%s} - ", commandList2[k-1]);
        }
        token_len2 = expand_wildcards(&commandList2, token_len2);
        //printf("\n");
        command_failed = my_pipe(commandList1, token_len1, commandList2, token_len2);

        for(int i=0;i<token_len1;i++){
            free(commandList1[i]);
        }
        free(commandList1);
        free(tmpToken1);


        for(int i=0;i<token_len2;i++){
            free(commandList2[i]);
        }
        free(commandList2);
        free(tmpToken2);
    }

    // no pipe
    else{
        char** tmpList = getTokens(full_command);
        int commandListSize = atoi(tmpList[0]) + 1;
        if(commandListSize <= 1){
            free(tmpList);
            perror("ERROR");
            return EXIT_FAILURE;
        }
        char **commandList = malloc(commandListSize * sizeof(char*));
        for(int k=1;k<commandListSize+1;k++){
            commandList[k-1] = tmpList[k];
        }
        commandListSize = expand_wildcards(&commandList, commandListSize);
        //check builtIn commands
        if(checkBuiltIn(commandList) == TRUE){
            //printf("BUILT IN!\n");
            command_failed = runBuiltIn(commandList, commandListSize, -1);
        }
        //fork otherwise
        else{
            int pid = fork();
            if (pid < 0) {
                perror("ERROR: fork failed\n");
                command_failed = TRUE;
            }
            //child
            else if(pid == 0){
                redirection(commandList, commandListSize);
                checkDirs(commandListSize, commandList);
            }
            //parent
            else{
                int wstatus;
                wait(&wstatus);
                if (WIFEXITED(wstatus)) {
                    // child exited normally
                    //printf("child exited with %d\n", WEXITSTATUS(wstatus));
                    command_failed = WEXITSTATUS(wstatus);
                }
            }
        }
        //printf("[%d]\n",commandListSize);
        for(int i=0;i<commandListSize-1;i++){
            free(commandList[i]);
        }
        free(commandList);
        free(tmpList);
    }
    if(command_failed){
        perror("ERROR");
    }
    return command_failed;
}

int main(int argc, char **argsv){

    char cwd[1024];
    curPathDir = getcwd(cwd, sizeof(cwd));
    //printf("%s",curPathDir);

    int fp;
    char new_buff[BUFF_SIZE];
    int bytes_read;
    int resize;
    char *buff = malloc(10);
    buff[0] = '\0';
    int cur_buff_size = 0;
    int offset = 0;
    int prev_command_failed = FALSE;
    if(argc == BATCH_MODE){
        fp = open(argsv[1], O_RDONLY);
        // check if the file was successfully opened
        if(fp == -1){
            perror("Error: file not found\n");
            free(buff);
            return EXIT_FAILURE;
        }
        bytes_read = read(fp, new_buff, BUFF_SIZE);
    }
    else if(argc == INTERACTIVE_MODE){
        fp = 0;
        printf("Welcome to my shell!\n");
        printf("mysh> ");
        fflush(stdout);
        bytes_read = read(fp, new_buff, BUFF_SIZE);
    }
    //main loop
    while((bytes_read) > 0){
        if(argc == INTERACTIVE_MODE){
            fflush(stdin);
        }
        resize = TRUE;
        int i;
        for (i = 0; i < BUFF_SIZE; i++){
            if(new_buff[i] == '\0'){
                resize = FALSE;
                break;
            }
            if(new_buff[i] == '\n'){
                new_buff[i] = '\0';
                resize  = FALSE;
                offset += i + 1;
                off_t position = lseek(fp, 0, SEEK_CUR);
                lseek(fp, offset - position, SEEK_CUR);
                position = lseek(fp, 0, SEEK_CUR);
                break;
            }
        }
        
        buff = realloc(buff, cur_buff_size + BUFF_SIZE);
        strncat(buff, new_buff, i+1);
        if(resize == TRUE){
            cur_buff_size += BUFF_SIZE;     
        }
        else{
            cur_buff_size = 0;
            prev_command_failed = tryExecute(curPathDir, buff);
            buff[0] = '\0';
            resize = TRUE;
        }
        if(argc == INTERACTIVE_MODE){
            if(prev_command_failed == TRUE){
                printf("!mysh> ");
            }
            else{
                printf("mysh> ");
            }
            fflush(stdout);
        }
        bytes_read = read(fp, new_buff, BUFF_SIZE);
    }
    free(buff);
    close(fp);
    return EXIT_SUCCESS;
}