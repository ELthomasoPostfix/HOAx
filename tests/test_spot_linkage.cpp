#include <iostream>
#include <spot/misc/version.hh>

int main()
{
  std::cout << "Test proper Spot Linkage!\nThis is Spot " << spot::version() << ".\n";
  return 0;
}