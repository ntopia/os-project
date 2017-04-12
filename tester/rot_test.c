#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/unistd.h>
#include "test.h"


int i, j;
char cmd[100];
const char* helps = "* help\t:\n"
"    e : end test\n"
"    S a : set_rotation( a );\n"
"    RL d r : rotlock_read( d, r );\n"
"    RU d r : rotunlock_read( d, r );\n"
"    WL d r : rotlock_write( d, r );\n"
"    WU d r : rotunlock_write( d, r );\n";

void prt_info(char* cmd, int deg, int ran){
    printf("* exec\t: [%s]\tDegree %d\tRange %d\n", cmd, deg, ran);
}

void prt_returns(long ret){
    if (ret < 0)
        printf("\treturn with error value\t: %d, %s\n", errno, strerror(errno));
    else 
        printf("\treturn value\t: %ld\n", ret);
}


int main(){
    while(1){
        printf("* input\t: ");
        scanf("%s", cmd);

        for (int i = 0; cmd[i] != '\0'; i++){
            if (cmd[i] >= 'a') cmd[i] -= 32;
        }
     
        if (cmd[0] == 'E') {
            printf("* exit\n");
            break;
        } else if (cmd[0] == 'H') {
            printf("%s", helps);
            continue;
        }
        
        long ret = 0;
        if(cmd[0] == 'S'){
            scanf("%d", &i);
            printf("* exec\t: [Set Rotation]\tDegree  %d\n", i);

            ret = syscall(__NR_set_rotation, i);

        } else{
            scanf("%d %d", &i, &j);
            if(strncmp(cmd, "RL", 2) == 0){
                prt_info("Read Lock", i, j);
                ret = syscall(__NR_rotlock_read, i, j);
            } else if (strncmp(cmd, "RU", 2) == 0){
                prt_info("Read Unlock", i, j);
                ret = syscall(__NR_rotunlock_read, i, j);
            } else if (strncmp(cmd, "WL", 2) == 0){
                prt_info("Write Lock", i, j);
                ret = syscall(__NR_rotlock_write, i, j);
            } else if (strncmp(cmd, "WU", 2) == 0){
                prt_info("Write Unlock", i, j);
                ret = syscall(__NR_rotunlock_write, i, j);
            } else {
                printf("* invalid command. type for \n");
                continue;
            }
        }

        prt_returns(ret);

    }
    return 0;
}
