#include <iostream>

// Function overloads
void foo(int a) { std::cout << "foo(int)\n"; }

void foo(double a) { std::cout << "foo(double)\n"; }

int main() {
  const char *str = "hello";
  const char *p = str;
  const char *p1 = str + 3;

  int len = p1 - p;
  printf("p %s p1 %s len %d\n", p, p1, len);

  int len1 = p - p1;
  printf("%d len1 ", len1);
  return 0;
}