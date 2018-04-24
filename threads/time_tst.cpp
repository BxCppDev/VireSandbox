#include <iostream>
#include <chrono>
#include <string>

#include "time_tools.h"

int main () {

   auto t0  = std::chrono::system_clock::now ();    //  begin of prog
   auto t1  = t0 + std::chrono::seconds      (1);   //  session start time
   auto d1  = std::chrono::seconds           (11);  //  session reservation duration
   auto t2  = t1 + d1;                              //  session end time (at max)

   std::chrono::duration<double> d1bis = t2 - t1;

   std::cout << "now = " << time_point_str (t0)  << std::endl;
   std::cout << "t1  = " << time_point_str (t1)  << std::endl;
   std::cout << "t2  = " << time_point_str (t2)  << std::endl;
   std::cout << "d1  = " << d1.count () << " s"  << std::endl;
   std::cout << "t2 < t1 ? " << (t2 < t1)        << std::endl;
   std::cout << "t1 < t2 ? " << (t1 < t2)        << std::endl;

   std::cout << "d1bis = " << d1bis.count () << " s" << std::endl;

   return EXIT_SUCCESS;
}

