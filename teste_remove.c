#include <stdio.h>
int main() {
  if (remove("HELLO.txt") == -1)
  perror("Error in deleting a file");
  //getch();
return 0;
}