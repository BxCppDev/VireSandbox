#ifndef SESSION_H
#define SESSION_H

#include <chrono>
#include <thread>
#include <string>
#include <future>
#include <atomic>

#include "use_case.h"


class session_t {

public :

   enum state_t {ERROR,
                 READY,
                 RUNNING,
                 STOPPED,
                 KILLED,
                 COMPLETED};

   static std::string state_str (state_t state);

   session_t  (use_case_t *                           u_case,
               std::chrono::system_clock::time_point  reservation_begin,
               std::chrono::system_clock::time_point  reservation_end);

   state_t get_state ();
   state_t start     ();
   void    stop      ();

private :

   use_case_t *                          use_case;
   std::chrono::system_clock::time_point t_begin;
   std::chrono::system_clock::time_point t_end;
   std::chrono::duration<double>         d_reservation;
   state_t                               state;
   std::atomic <bool>                    stop_requested;

   enum stage_typ {INIT, WITER, FINISH};

   use_case_t::completion_t stage_perform (stage_typ typ);

   void init_stage     (std::promise <use_case_t::completion_t> * complet);
   void work_iteration (std::promise <use_case_t::completion_t> * complet);
   void finish_stage   (std::promise <use_case_t::completion_t> * complet);

};

#endif

