#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

//_________________Types:_________________//


struct String {
/* String is a dynamic string */
    char* data;
    int capacity;
    int size;
};

struct Cmd {
/* Cmd contains "simple command" (echo a, cat, ls..) */
    char** word;
    int capacity;
    int size;
};

struct Command {
/* Command contains 1 paralleled process with its pipe, input-output files */
    char* in_file;
    bool replacement;
    char* out_file;
    bool background;
    char*** data;
    int size;
    int capacity;
};

struct Input {
/* Input contains all paralleled processes */
    struct Command* data;
    char* inp_error;
    char* run_error;
    int size;
    int capacity;
    bool end;
};

//_________________Flags:_________________//

bool EndOfFile(int c) {
    return c == '\n' || c == EOF;
}

bool EndOfWord(int c) {
    return c == ' ' || c == '|' || c == '&' || c == '>' || c == '<';
}

bool IsStrEq(char* s1, char* s2) {
    while(*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

//_________________String:_________________//

void InitString(struct String* s) {
/* initialization of string */
    s->size= 0;
    s->capacity = 16;
    s->data = (char*) malloc(s->capacity * sizeof(char));
}

void AppendString(struct String* s, char c) {
/* adding one symbol to the string */
    if(s->size == s->capacity - 1) {
        s->capacity *= 2;
        s->data = (char*) realloc(s->data, s->capacity * sizeof(char));
    }
    (s->data)[s->size] = c;
    (s->size)++;
}

char* GetString(struct String* s) {
/* get c-string */
    (s->data)[s->size] = '\0';
    return s->data;
}

void FreeString(struct String* s) {
/* free memory allocated for String.data */
    free(s->data);
}

//_________________Simple_Command:_________________//

void InitCmd(struct Cmd* scmd) {
/* initialization of new simple command */
    scmd->size = 0;
    scmd->capacity = 2;
    scmd->word = (char**) malloc(scmd->capacity * sizeof(char*));
}

void AppendCmd(struct Cmd* scmd, struct String* s) {
/* adding new word to the command */
    if(scmd->size == scmd->capacity - 1) {
        scmd->capacity *= 2;
        scmd->word = (char**) realloc(scmd->word, scmd->capacity * sizeof(char*));
    }
    (scmd->size)++;
    (scmd->word)[scmd->size - 1] = GetString(s);
    (scmd->word)[scmd->size] =  NULL;
}

void PrintCmd(char** scmd) {
/* printing simple command */
    for(int i = 0; scmd[i] != NULL; i++) {
        printf("%s_", scmd[i]);
    }
    printf("\n");
}

void FreeCmd(struct Cmd* scmd) {
/* free memory allocated for Cmd.word */
    for(int i = 0; i < scmd->size; i++) {
        free(scmd->word[i]);
    }
    free(scmd->word);
}

//_________________Command:_________________//

void InitCommand(struct Command* cmd) {
/* initialization of new paralleled process */
    cmd->in_file = NULL;
    cmd->replacement = true;
    cmd->out_file = NULL;
    cmd->background = false;
    cmd->size = 0;
    cmd->capacity = 2;
    cmd->data = (char***) malloc((cmd->capacity) * sizeof(char**));
}

void AppendCommand(struct Command* cmd, struct Cmd* scmd) {
/* adding new command to the pipe */
    (cmd->size)++;
    if(cmd->size == cmd->capacity) {
        cmd->capacity *= 2;
        cmd->data = (char***) realloc(cmd->data, (cmd->capacity) * sizeof(char**));
    }
    (cmd->data)[cmd->size - 1] = scmd->word;
}

void PrintCommand(struct Command* cmd) {
/* printing paralleled process */
    printf("\n");
    printf("Input_file: %s\n", cmd->in_file);
    printf("File_replacement: %s\n", (cmd->replacement == true)? "TRUE" : "FALSE");
    printf("Output_file: %s\n", cmd->out_file);
    printf("Background: %s\n", (cmd->background == true)? "TRUE" : "FALSE");
    printf("Pipe: \n");
    for(int i = 0; i < cmd->size; i++) {
        printf("%d) ", i);
        PrintCmd(cmd->data[i]);
    }
    printf("\n");
}

void FreeCommand(struct Command* cmd) {
/* free memory allocated for Command.data */
    if(cmd->in_file != NULL)
        free(cmd->in_file);
    if(cmd->out_file != NULL)
        free(cmd->out_file);
    for(int i = 0; i < cmd->size; i++) {
        int j = 0;
        while(((cmd->data)[i])[j] != NULL) {
            free(((cmd->data)[i])[j]);
            j++;
        }
        free((cmd->data)[i]);
    }
    free(cmd->data);
}

//_________________Input:_________________//

void InitInput(struct Input* inp) {
/* initialization of whole input line */
    inp->size = 0;
    inp->inp_error = NULL;
    inp->run_error = NULL;
    inp->capacity = 2;
    inp->data = (struct Command*) malloc(inp->capacity * sizeof(struct Command));
    inp->end = false;
}

void AppendInput(struct Input* inp, struct Command* cmd) {
/* adding one more paralleled process */
    (inp->size)++;
    if(inp->size == inp->capacity) {
        inp->capacity *= 2;
        inp->data = (struct Command*) realloc(inp->data, inp->capacity * sizeof(struct Command));
    }
    inp->data[inp->size - 1] = *cmd;
}

void PrintInput(struct Input* inp) {
/* printing all paralleled processes */
    for(int i = 0; i < inp->size; i++) {
        printf("command %d", i);
        PrintCommand(&(inp->data[i]));
    }
    
}

void FreeInput(struct Input* inp) {
/* free memory allocated for Input.data */
    for(int i = 0; i < inp->size; i++)
        FreeCommand(&(inp->data[i]));
    free(inp->data);
}

//_________________Helpful_functions:_________________//

void InitAll(struct Input* inp, struct Command* cmd, struct Cmd* scmd, struct String* s) {
/* initialization of everything by 1 func :) */
    InitInput(inp);
    InitCommand(cmd);
    InitCmd(scmd);
    InitString(s);
}

void AddWord(struct String* s, struct Cmd* scmd) {
/* adding new word to a command */
    if(s->size != 0) {
        AppendCmd(scmd, s);
        InitString(s);
    }
}

void AddPipe(struct Command* cmd, struct Cmd* scmd, struct String* s) {
/* adding new command to the pipe */
    if(scmd->size != 0) {
        AppendCommand(cmd, scmd);
        InitCmd(scmd);
    }
}

//_________________Input_Parsing:_________________//

void QuotMarks(struct String* s, int* c, struct Input* inp) {
/* text in quotations */
    *c = getchar();
    while(*c != '"') {
        if(*c == '\\') {
            int k = getchar();
            if(k != '\\' && k != '"')
                AppendString(s, *c);
            *c = k;
        }
        if(*c == EOF) {
            inp->inp_error = "Quotation mark is missed";
            break;
        }
        AppendString(s, *c);
        *c = getchar();
    }
}

void Apostrophe(struct String* s, int* c, struct Input* inp) {
/* text in apostrophes */
    *c = getchar();
    while(*c != '\'') {
        if(*c == EOF) {
            inp->inp_error = "Apostrophe is missed";
            break;
        }
        AppendString(s, *c);
        *c = getchar();
    }
}

void ReadWord(struct String* s, int* c, struct Input* inp) {
/* reading text (including quots) until EndOfWord or EndOfFile */
    while(*c == ' ')
        *c = getchar();
    while(!EndOfFile(*c) && !EndOfWord(*c)) {
        switch(*c) {
            case '"':
                QuotMarks(s, c, inp);
                if(!EndOfFile(*c) && !EndOfWord(*c)) {
                    *c = getchar();
                    if((EndOfFile(*c) || EndOfWord(*c)) && (s->size == 0))
                        // case of empty string
                        s->size = -1;
                }
                break;
            case '\'':
                Apostrophe(s, c, inp);
                if(!EndOfFile(*c) && !EndOfWord(*c)) {
                    *c = getchar();
                    if((EndOfFile(*c) || EndOfWord(*c)) && (s->size == 0))
                        // case of empty string
                        s->size = -1;
                }
                break;
            case '\\':
                *c = getchar();
                if(!EndOfFile(*c)) {
                    AppendString(s, *c);
                    *c = getchar();
                }
                break;
            default:
                AppendString(s, *c);
                *c = getchar();
                break;
        }
    }
}

void ChangeInpFile(struct Command* cmd, int* c, struct Input* inp) {
    struct String s;
    InitString(&s);
    ReadWord(&s, c, inp);
    if(s.size == 0) {
        inp->inp_error = "Argument for '<' is missed";
        FreeString(&s);
    } else {
        if(cmd->in_file != NULL) {
            free(cmd->in_file);
        } else {
            cmd->in_file = GetString(&s);
        }
    }
}

void ChangeOutpFile(struct Command* cmd, int* c, struct Input* inp) {
/* changing output file */
    struct String s;
    InitString(&s);
    ReadWord(&s, c, inp);
    if(s.size == 0) {
        inp->inp_error = "Argument for '>' or '>>' is missed";
        FreeString(&s);
    } else {
        if(cmd->out_file != NULL) {
            free(cmd->out_file);
        } else {
            cmd->out_file = GetString(&s);
        }
    }
}

void ReadInput(struct Input* inp) {
/* reading input */
    struct Command cmd;
    struct Cmd scmd;
    struct String s;
    InitAll(inp, &cmd, &scmd, &s);
    int c = getchar();
    while(!EndOfFile(c)) {
        switch(c) {
            case '|':
                c = getchar();
                break;
            case '&':
                cmd.background = true;
                AppendInput(inp, &cmd);
                InitCommand(&cmd);
                c = getchar();
                break;
            case '<':
                c = getchar();
                ChangeInpFile(&cmd, &c, inp);
                break;
            case '>':
                if ((c = getchar()) == '>') {
                    cmd.replacement = false;
                    c = getchar();
                }
                ChangeOutpFile(&cmd, &c, inp);
                break;
            default:
                ReadWord(&s, &c, inp);
                AddWord(&s, &scmd);
                if(c != ' ')
                    AddPipe(&cmd, &scmd, &s);
                break;
        }
    }
    if(s.size != 0) {
        AppendCmd(&scmd, &s);
        AppendCommand(&cmd, &scmd);
    } else {
        FreeString(&s);
        FreeCmd(&scmd);
    }
    if(cmd.size > 0) {
        AppendInput(inp, &cmd);
    } else {
        FreeCommand(&cmd);
    }
    if(c == EOF) {
        inp->end = true;
        printf("\n");
    }
}

void Invitation() {
    printf("\033[1;34m$\033[0m ");
    fflush(stdout);
}

//_________________Useful_functions_for_Input_Run:_________________//

int* InitPipes(struct Input* inp, int n) {
/* Initialize (n - 1) * 2 pipes and return addres of the first */
    n = (n - 1) * 2;
    int* pipe_channel = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i += 2) {
        if(pipe(pipe_channel + i) == -1) {
            inp->run_error = "Failed to pipe channel";
            return NULL;
        }
    }
    return pipe_channel;
}

void ClosePipes(int* pipes, int n) {
/* Close (n - 1) * 2 pipes beginning with int* pipes */
    for(int i = 0; i < (n - 1) * 2; i++) {
        close(pipes[i]);
    }
}

void CloseProcPipes(struct Input* inp, int i, int* pipe_channel, int k) {
/* Close pipes used by finished proccess*/
    if(k == 0) {
        close(pipe_channel[1]);
    } else if(k == inp->data[i].size - 1) {
        close(pipe_channel[2 * (k - 1)]);
    } else {
        close(pipe_channel[2 * k + 1]);
        close(pipe_channel[2 * (k - 1)]);
    }
}

void Execute(struct Input* inp, int i, int j) {
/* Execvp with errors handler */
    execvp(inp->data[i].data[j][0], inp->data[i].data[j]);
    inp->run_error = (char*) malloc(256);
    sprintf(inp->run_error, "Failed to run command: %s", inp->data[i].data[j][0]);
    inp->end = true;
}

bool DupCh(struct Input* inp, int new_ch, int old_ch) {
/* Dup2 with errors handler */
    if(dup2(new_ch, old_ch) == -1) {
        inp->run_error = "Failed to dup channel";
        return false;
    } else {
        close(new_ch);
        return true;
    }
}

bool ChangeDescrInPipe(struct Input* inp, int* pipe_channel, int i, int j) {
/* Change STDIN and STDOUT for proccesses in pipe */
    if(j == 1) { //first proccess
        if(!DupCh(inp, pipe_channel[1], 1)) {
            return false;
        }
    } else if(j == inp->data[i].size) { //last proccess
        if(!DupCh(inp, pipe_channel[2 * (j - 2)], 0)) {
            return false;
        }
    } else { //all other proccesses
        if(!DupCh(inp, pipe_channel[2 * j - 1], 1)) {
            return false;
        }
        if(!DupCh(inp, pipe_channel[2 * (j - 2)], 0)) {
            return false;
        }
    }
    return true;
}

void TheWalkingDead() {
    while(waitpid(-1, NULL, WNOHANG) > 0) {}; // clean all existing zombies
}


//_________________Input_Run:_________________//

bool RunInputEmbedded(struct Input* inp, int i) {
/*  running commands which don't need fork()
    RunInputEmbedded == true <=> end of RunInput  */
    if(IsStrEq(inp->data[i].data[0][0], "exit")) {
        if(inp->data[i].data[0][1] != NULL) {
            inp->inp_error = "Exit doesn't expect arguments";
        } else {
            inp->end = true;
        }
        return true;
    }
    if(IsStrEq(inp->data[i].data[0][0], "cd")) {
        if(inp->data[i].data[0][1] == NULL) {
            inp->inp_error = "Argument for cd is missed";
        } else {
            if(chdir(inp->data[i].data[0][1]) != 0) {
                inp->run_error = "Failed to change directory";
            }
        }
        return true;
    }
    return false;
}

bool PipeEmbeddedCheck(struct Input* inp, int i) {
/* Checking if user is trying to perform cd or exit in pipe */
    if(inp->data[i].size > 1) {
        for(int j = 0; j < inp->data[i].size; j++) {
            if(IsStrEq("cd", inp->data[i].data[j][0]) || IsStrEq("exit", inp->data[i].data[j][0])) {
                inp->run_error = "Can't execute cd or exit in pipe";
                return true;
            }
        }
    }
    return false;
}

bool ChangeStdOut(struct Input* inp, struct Command* cmd) {
    int outfile;
    if(!cmd->replacement) {
        outfile = open(cmd->out_file, O_CREAT | O_RDWR | O_APPEND, 0666);
    } else {
        outfile = open(cmd->out_file, O_CREAT | O_RDWR | O_TRUNC, 0666);
    }
    if(outfile == -1) {
        inp->run_error = "Failed to open output file";
        return false;
    }
    return DupCh(inp, outfile, 1);
}

bool ChangeStdIn(struct Input* inp, struct Command* cmd) {
    int infile = open(cmd->in_file, O_RDONLY);
    if(infile == -1) {
        inp->run_error = "Failed to open input file";
        return false;
    }
    return DupCh(inp, infile, 0);
}

bool RunPipe(struct Input* inp, int i) {
/* Run commands in pipe */
    int pipe_child = -1;
    int* pipe_childs = (int*) malloc(inp->data[i].size * sizeof(int));
    int* pipe_channel = InitPipes(inp, inp->data[i].size);
    // pipe_channel[0] for read
    // pipe_channel[1] for write
    if(pipe_channel == NULL) {
        return false;
    }
    // pipe execution
    int saved_stdout = dup(1);
    for(int j = 1; j <= inp->data[i].size; j++) {
        pipe_child = fork();
        if(pipe_child == -1) {
            inp->run_error = "Failed to create child proccess";
            return false;
        }
        if(pipe_child == 0) { // child proccess: execute new command in pipe
            if(!ChangeDescrInPipe(inp, pipe_channel, i, j)) {
                return false;
            }
            ClosePipes(pipe_channel, inp->data[i].size);
            Execute(inp, i, j - 1);
            DupCh(inp, saved_stdout, 1);
            return false;
        } else { // parent proccess: add pipe_childPID to pipe_childs
            pipe_childs[j - 1] = pipe_child;
        }
    }
    ClosePipes(pipe_channel, inp->data[i].size);
    for(int k = 0; k < inp->data[i].size; k++) {
        waitpid(pipe_childs[k], NULL, 0);
    }
    free(pipe_channel);
    free(pipe_childs);
    return true;
}

void RunInput(struct Input* inp) {
    if(inp->data->size != 0) {
        int i = 0;
        int child = -1;
        int* childs = (int*) malloc(inp->size * sizeof(int));
        // paralleled proccesses
        while(i < inp->size) {
            if(PipeEmbeddedCheck(inp, i) == true || inp->data[i].size == 0) {
                return;
            }
            if(RunInputEmbedded(inp, i) == true) { // running commands which don't need fork();
                if(inp->end) {
                    return;
                } else {
                    i++;
                    continue;
                }
            }
            child = fork();
            if(child == -1) {
                inp->run_error = "Failed to create child proccess";
                return;
            }
            if(child == 0) { // child proccess: leave loop and perform actions
                break;
            } else { // parent proccess: add childPID to childs and continue loop
                childs[i] = child;
                i++;
            }
        }
        if(child == 0) {
            inp->end = true;
            // change STDOUT
            if(inp->data[i].out_file != NULL) {
                if(ChangeStdOut(inp, &(inp->data[i])) == false) {
                    return;
                }
            }
            // change STDIN
            if(inp->data[i].in_file != NULL) {
                if(ChangeStdIn(inp, &(inp->data[i])) == false) {
                    return;
                }
            }
            // pipe check
            if(inp->data[i].size > 1) {
                if(RunPipe(inp, i) == false) {
                    return;
                }
            } else {
                Execute(inp, i, 0);
            }
        } else {
            for(int s = 0; s < inp->size; s++) {
                if(inp->data[s].background != true) {
                    waitpid(childs[s], NULL, 0);
                }
            }
            free(childs);
        }
    }
}

//_________________Printing_errors:_________________//

void PrintErrors(struct Input* inp) {
    if(inp->data->size != 0) {
        if(inp->inp_error != NULL)
            printf("INPUT_ERROR: %s\n", inp->inp_error);
        if(inp->run_error != NULL) {
            printf("RUN_ERROR: %s\n", inp->run_error);
            free(inp->run_error);
        }
    }
}

int main() {
    struct Input inp;
    do {
        TheWalkingDead();
        Invitation();
        ReadInput(&inp);
        RunInput(&inp);
        PrintErrors(&inp);
        FreeInput(&inp);
    } while(!inp.end);
    return 0;
}


/*
 Compilation:
 cd /Users/lesenka/Documents/Универ/2курс/Прога/Си/task1/task1
 gcc -Wall -g main.c -o main.o
 
 Memory leaks:
 leaks -atExit -- ./main.o
*/

