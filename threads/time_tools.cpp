#include "time_tools.h"

//#include <iostream>
#include <ctime>



std::string time_point_str (const std::chrono::system_clock::time_point& tp)
{
   std::time_t t  = std::chrono::system_clock::to_time_t (tp);
   std::string ts = std::ctime (&t);
   ts.resize (ts.size () - 1);
   return ts;
}


