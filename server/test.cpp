#include <iostream>

// Function overloads
void foo(int a) { std::cout << "foo(int)\n"; }

void foo(double a) { std::cout << "foo(double)\n"; }

int main() {
  foo(10);    // Calls foo(int)
  foo(10.5);  // Calls foo(double)
  foo('a');   // Ambiguous, can be converted to either int or double
  return 0;
}