#include <iostream>
#include <chrono>

#include "use_case.h"

int main () {

   use_case_t::completion_t complet;
   use_case_t               uc ("uc_test",
                                std::chrono::seconds (2),
                                std::chrono::seconds (6),
                                std::chrono::seconds (2));

   std::cout << "use case name           " << uc.get_name ()                                    << std::endl;
   std::cout << "         stage          " << use_case_t::stage_str (uc.get_stage ())           << std::endl;
   std::cout << "         completion     " << use_case_t::completion_str (uc.get_completion ()) << std::endl;
   std::cout << "         init max (s)   " << uc.get_init_stage_duration_max ().count ()        << std::endl;
   std::cout << "         work max (s)   " << uc.get_work_stage_duration_max ().count ()        << std::endl;
   std::cout << "         finish max (s) " << uc.get_finish_stage_duration_max ().count ()      << std::endl;
   std::cout << std::endl;

   complet = uc.init_stage ();
   std::cout << "init stage =" << use_case_t::completion_str (complet) << std::endl;
   std::cout << std::endl;

   complet = uc.work_iteration ();
   while (complet == use_case_t::TO_BE_CONTINUED) complet = uc.work_iteration ();
   std::cout << "work stage =" << use_case_t::completion_str (complet) << std::endl;
   std::cout << std::endl;

   complet = uc.finish_stage ();
   std::cout << "finish stage =" << use_case_t::completion_str (complet) << std::endl;
   std::cout << std::endl;

   return EXIT_SUCCESS;
}

