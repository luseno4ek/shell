#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct String {
/* String is a dynamic string */
    char* data;
    int capacity;
    int size;
};

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

int* InitPipe(int n) {
    n = (n - 1) * 2;
    int* pipe_channel = (int*) malloc(n * sizeof(int));
    for(int i = 0; i < n; i += 2) {
        if(pipe(pipe_channel + i) == -1) {
            printf("Fail to pipe the channel");
            return NULL;
        }
    }
    return pipe_channel;
}

void ClosePipe(int* pipe_channel, int n) {
    n = (n - 1) * 2;
    for(int i = 0; i < n; i++) {
        close(pipe_channel[i]);
    }
}

int main() {
    const int n = 2;
    int pipes[n];
    int* pipe_channel = InitPipe(n);
    if(pipe_channel == NULL) {
        return 1;
    }
    for(int i = 1; i <= n; i++) {
        int child = fork();
        if(child == -1) {
            printf("Failed to create child");
            return 1;
        }
        if(child == 0) {
        // child proccess
            printf("CREATE: proccess %d with PID %d\n", i, getpid());
            switch(i) {
                case 1: // first proccess
                    if(dup2(pipe_channel[1], 1) == -1) {
                        printf("Failed to dup");
                        return 1;
                    }
                    break;
                case n: // last proccess
                    if(dup2(pipe_channel[2 * (i - 2)], 0) == -1) {
                        printf("Failed to dup");
                        return 1;
                    }
                    break;
                default: // all other proccesses
                    if(dup2(pipe_channel[2 * (i - 2)], 0) == -1) {
                        printf("Failed to dup");
                        return 1;
                    }
                    if(dup2(pipe_channel[2 * i - 1], 1) == -1) {
                        printf("Failed to dup");
                        return 1;
                    }
                    break;
            }
            ClosePipe(pipe_channel, n);
            struct String s;
            InitString(&s);
            int c = getchar();
            int check = '\n';
            while(c != check) {
                AppendString(&s, c);
                c = getchar();
            }
            printf("%d: %s\n", getpid(), GetString(&s));
            FreeString(&s);
            return 0;
        } else {
            pipes[i - 1] = child;
        }
    }
    for(int i = 0; i < n; i++) {
        printf("-CHECK: proccess with PID %d \n", pipes[i]);
        printf("--FINISH: proccess with PID %d\n", waitpid(pipes[i], NULL, 0));
    }
    return 0;
}
