#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include <stdio.h>


void process(char &buf)
{
   char str[256];

   int i = 0, j = 0;
   while (buf + i != '\0')
   {
      // Tim ky tu khac dau cach dau tien tu vi tri hien tai
      while (*(buf+i) == ' ' && buf+i != '\0') 
      {
         buf + i = '\0';
         i++;
         if (buf+i == '\0')   
         break;
      }
         

      // Copy 1 tu sang xau dich
      int first = 1;
      while (buf+i != ' ' && buf+i != '\0')
      {
         if (first)
         {
            toupper(buf + i++);
            first = 0;
         }
         else
         {
            tolower(buf + i++);
         }
      }
            
      if (buf+i == '\0')
         break;

      // Copy 1 dau cach sau tu vua copy
      
   }

   // Kiem tra neu tu chua dau cach cuoi thi xoa
   if (str[j - 1] == ' ')
      str[j - 1] = '\0';
   else
      str[j] = '\0';

}

int main() {
   char buf[256] = " adfaegawg sdfa wda    ";
   printf("%s", buf);
}