#include "use_case.h"

#include <iostream>
#include <thread>

std::string use_case_t::completion_str (use_case_t::completion_t complet) {
   switch (complet) {
      case use_case_t::TO_BE_CONTINUED : return "TO_BE_CONTINUED";
      case use_case_t::COMPLETED       : return "COMPLETED";
      case use_case_t::ERROR           : return "ERROR";
      case use_case_t::STOPPED         : return "STOPPED";
      case use_case_t::TIMEOUT         : return "TIMEOUT";
      default                          : return "WHAT?";
   }
}

std::string use_case_t::stage_str (use_case_t::stage_t stage) {
   switch (stage) {
      case use_case_t::NEW          : return "NEW";
      case use_case_t::INITIALIZING : return "INITIALIZING";
      case use_case_t::INITIALIZED  : return "INITIALIZED";
      case use_case_t::WORKING      : return "WORKING";
      case use_case_t::WORKED       : return "WORKED";
      case use_case_t::FINISHING    : return "FINISHING";
      case use_case_t::FINISHED     : return "FINISHED";
      default                       : return "WHAT?";
   }
}

use_case_t::use_case_t (std::string          ucase,
                        std::chrono::seconds init_max,
                        std::chrono::seconds work_max,
                        std::chrono::seconds finish_max) {
   current_stage          = NEW;
   current_completion     = TO_BE_CONTINUED;
   name                   = ucase;
   init_duration_max      = init_max;
   work_duration_max      = work_max;
   finish_duration_max    = finish_max;
   nb_iterations          = work_duration_max.count () - 1;
   iter_duration          = std::chrono::seconds (1);
   current_work_iter      = 0;
   current_completion     = COMPLETED;
   work_slowly            = false;
}

std::string use_case_t::get_name () {
   return name;
}

use_case_t::stage_t use_case_t::get_stage () {
   return current_stage;
}

use_case_t::completion_t use_case_t::get_completion () {
   return current_completion;
}

std::chrono::seconds use_case_t::get_duration_max () {
   return init_duration_max + work_duration_max + finish_duration_max;
}

std::chrono::seconds use_case_t::get_init_stage_duration_max () {
   return init_duration_max;
}

std::chrono::seconds use_case_t::get_work_stage_duration_max () {
   return work_duration_max;
}

std::chrono::seconds use_case_t::get_finish_stage_duration_max () {
   return finish_duration_max;
}

use_case_t::completion_t use_case_t::init_stage () {
   if (enter_stage (INITIALIZING)) {
      std::this_thread::sleep_for (init_duration_max - std::chrono::seconds (1));
      quit_stage ();
   }
   return current_completion;
}

use_case_t::completion_t use_case_t::work_iteration () {
   if (enter_stage (WORKING)) {
      if (work_slowly) std::this_thread::sleep_for (iter_duration + std::chrono::seconds (3));
      else             std::this_thread::sleep_for (iter_duration);
      quit_stage ();
   }
   return current_completion;
}

use_case_t::completion_t use_case_t::finish_stage () {
   if (enter_stage (FINISHING)) {
      std::this_thread::sleep_for (finish_duration_max - std::chrono::seconds (1));
      quit_stage ();
   }
   return current_completion;
}

void use_case_t::set_too_slow () {
   work_slowly = true;
}

bool use_case_t::enter_stage (use_case_t::stage_t stage) {
   bool valid = false;
   if (current_completion != ERROR   and
       current_completion != STOPPED and
       current_completion != TIMEOUT) {
      switch (stage) {
         case INITIALIZING :
            if (current_stage == NEW)  {
               current_stage      = INITIALIZING;
               current_completion = TO_BE_CONTINUED;
               valid              = true;
            }
            break;
         case WORKING :
            if ((current_stage == INITIALIZED and current_work_iter == 0)
                 or
                (current_stage == WORKING and current_work_iter < nb_iterations)) {
               current_stage      = WORKING;
               current_completion = TO_BE_CONTINUED;
               current_work_iter  = current_work_iter + 1;
               valid              = true;
            }
            break;
         case FINISHING :
            if (current_stage == WORKED) {
               current_stage      = FINISHING;
               current_completion = TO_BE_CONTINUED;
               valid              = true;
            }
            break;
         default :
            valid = false;
      }
   }
   if (not valid) current_completion = ERROR;
   return valid;
}

void use_case_t::quit_stage () {
   if (current_completion != ERROR   and
       current_completion != STOPPED and
       current_completion != TIMEOUT) {
      switch (current_stage) {
         case INITIALIZING :
            current_stage      = INITIALIZED;
            current_completion = COMPLETED;
            break;
         case WORKING :
            if (current_work_iter < nb_iterations) {
               current_stage      = WORKING;
               current_completion = TO_BE_CONTINUED;
            } else {
               current_stage      = WORKED;
               current_completion = COMPLETED;
            }
            break;
         case FINISHING :
            current_stage      = FINISHED;
            current_completion = COMPLETED;
            break;
         default :
            current_completion = ERROR;
      }
   }
}


// café du commerce 75015
// RER B + DEnfert_Rochereau +++
