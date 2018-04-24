#include <iostream>
#include <chrono>

#include "use_case.h"
#include "session.h"
#include "time_tools.h"

int main (int argc, char** argv) {

   const std::string slow          = "slow";
   const std::string stop          = "stop";
   bool              use_case_slow = false;
   bool              stop_request  = false;

   for (int a=0; a<argc; a++) {
      if (slow.compare (argv [a]) == 0) use_case_slow = true;
      if (stop.compare (argv [a]) == 0) stop_request  = true;
   }

   std::cout                                           << std::endl;
   std::cout                                           << std::endl;
   std::cout << "usage : "                             << std::endl;
   std::cout << "   " << argv [0] << " [slow][stop]"   << std::endl;
   std::cout                                           << std::endl;
   std::cout                                           << std::endl;
   std::cout << "   use case slow = " << use_case_slow << std::endl;
   std::cout << "   stop request  = " << stop_request  << std::endl;
   std::cout                                           << std::endl;
   std::cout                                           << std::endl;

   auto t0 = std::chrono::system_clock::now ();    //  begin of prog
   auto t1 = t0 + std::chrono::seconds      (1);   //  session start time
   auto d1 = std::chrono::seconds           (11);  //  session reservation duration
   auto t2 = t1 + d1;                              //  session end time (at max)

   use_case_t uc ("uc_test",
                  std::chrono::seconds (2),
                  std::chrono::seconds (6),
                  std::chrono::seconds (2));
   session_t  ss (&uc, t1, t2);

   if (use_case_slow) uc.set_too_slow ();

   std::cout << std::endl;

   std::cout << "now                     " << time_point_str (t0)                               << std::endl;
   std::cout << std::endl;

   std::cout << "session  case           " << uc.get_name ()                                    << std::endl;
   std::cout << "         begin          " << time_point_str (t1)                               << std::endl;
   std::cout << "         end            " << time_point_str (t2)                               << std::endl;
   std::cout << "         reservation    " << d1.count ()  << " s"                              << std::endl;
   std::cout << "         state          " << session_t::state_str (ss.get_state ())            << std::endl;
   std::cout << std::endl;

   std::cout << "use case name           " << uc.get_name ()                                    << std::endl;
   std::cout << "         stage          " << use_case_t::stage_str (uc.get_stage ())           << std::endl;
   std::cout << "         completion     " << use_case_t::completion_str (uc.get_completion ()) << std::endl;
   std::cout << "         init max (s)   " << uc.get_init_stage_duration_max ().count ()        << std::endl;
   std::cout << "         work max (s)   " << uc.get_work_stage_duration_max ().count ()        << std::endl;
   std::cout << "         finish max (s) " << uc.get_finish_stage_duration_max ().count ()      << std::endl;
   std::cout << std::endl;

   std::future <session_t::state_t> uc_future = std::async (std::launch::async, &session_t::start, &ss);

   std::this_thread::sleep_for (std::chrono::seconds (3));
   if (stop_request) ss.stop ();

   uc_future.wait ();
   std::cout << std::endl << "final session state = " << session_t::state_str (uc_future.get ()) << std::endl;

   auto tf = std::chrono::system_clock::now ();
   std::chrono::duration<double> sess_dur = tf - t1;

   std::cout << std::endl;
   std::cout << "now                     " << time_point_str (tf)         << std::endl;
   std::cout << "session duration        " << sess_dur.count ()  << " s"  << std::endl;
   std::cout << std::endl;

   return EXIT_SUCCESS;

}

