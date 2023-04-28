#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    char input[300];
    int fp = 0;
    // Read from standard input
    read(fp, input, 300);
    for(int i=1;i<argc;i++){
        printf("[%s]", argv[i]);
    }
    // Print to standard output
    printf("\nBAZ: %s", input);

    return 0;
}