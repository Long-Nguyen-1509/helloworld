
#include<stdlib.h>
#include<string.h>


int main() {
   char cmd[32];
   strcpy(cmd,"dir > out.txt");
   system(cmd);
   return 0;
}