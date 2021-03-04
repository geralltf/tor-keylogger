#include <stdio.h>

int main(void)
{
   char buf[4096];
   int x;
   
   while (fgets(buf, sizeof(buf)-2, stdin) != NULL) {
        for (x = 0; x < 128; ) {
            printf("0x%c%c, ", buf[x], buf[x+1]);
            if (!((x += 2) & 31)) printf("\n");
        }
   }
}


/* ref:         HEAD -> develop */
/* git commit:  910d6252770f1e517d9ed02dc0549a1d61dfe159 */
/* commit time: 2021-01-19 13:51:22 +0100 */
