#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "fs_dev.hpp"

using test_funct_t = bool (*) ();
std::stringstream help_str;
std::vector<test_funct_t> test_funcs;
// constexpr void register_func(const test_func_t func)
// {
//    test_funcs.push_back(func);

// }
#define REGISTER_FUNCTION(func)\
test_funcs.push_back(&func); \
help_str << "# " << test_funcs.size() << ":" << #func << std::endl;

int main(int argc, const char** argv)
{
   bool ret = false;
   size_t idx;
   std::cout << "#----------------- TEST SUITE -----------------#" << std::endl;
   help_str << "# Usage: ./test_suite <int>, where int:" << std::endl;
   REGISTER_FUNCTION(openDevice_write_and_wait_5s);
   REGISTER_FUNCTION(openDevice_write_and_read);

   if (argc != 2)
   {
      std::cout << help_str.str();
      return 0;
   }

   try
   {
      idx = std::stoi(argv[1]);
      ret = test_funcs.at(idx-1)();
   }
   catch(std::exception &e)
   {
      std::cout << e.what() << std::endl;
      std::cout << help_str.str();
   }

   return ret;
}