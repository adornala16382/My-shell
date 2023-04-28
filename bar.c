#include <stdio.h>

int main(int argc, char** argv) {

    // Print to standard output
    printf("BAR: %s\n", argv[1]);
    if(argc > 2){
        return 1;
    }
    return 0;
}