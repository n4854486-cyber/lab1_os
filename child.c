#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

int main(void){
    char buf[4098];
    char str[4098];
    size_t i = 0;
    char c;
    while(read(STDIN_FILENO, &c, 1) == 1){
        if(c == '\n' || i >= sizeof(buf)) {
            if (i == 0){
                break;
            } else{
                for (size_t j = 0; j < i; j++){
                    str[j] = buf[i - j - 1];
                }
            }
            write(STDOUT_FILENO, str, i);
            write(STDOUT_FILENO, "\n", 1);
            i = 0;
        } else {
            if(i < sizeof(buf) - 1) {
                buf[i] = c;
                i++;
            }
        }   
    }
}