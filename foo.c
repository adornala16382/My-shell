#include <stdio.h>
#include <unistd.h>

int main() {
    char input[300];
    int fp = 0;
    // Read from standard input
    read(fp, input, 300);

    // Print to standard output
    printf("You entered: %s\n", input);

    return 0;
}