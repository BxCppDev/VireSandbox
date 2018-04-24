#include "session.h"
#include "time_tools.h"

#include <iostream>
#include <pthread.h>
#include <thread>

std::string session_t::state_str (session_t::state_t state) {
   switch (state) {
      case session_t::ERROR     : return "ERROR";
      case session_t::READY     : return "READY";
      case session_t::RUNNING   : return "RUNNING";
      case session_t::STOPPED   : return "STOPPED";
      case session_t::KILLED    : return "KILLED";
      case session_t::COMPLETED : return "COMPLETED";
      default                   : return "WHAT?";
   }
}

session_t::session_t  (use_case_t *                           u_case,
                       std::chrono::system_clock::time_point  reservation_begin,
                       std::chrono::system_clock::time_point  reservation_end) {
   t_begin       = reservation_begin;
   t_end         = reservation_end;
   d_reservation = t_end - t_begin;
   if (u_case->get_stage ()      != use_case_t::NEW
       and
       u_case->get_completion () != use_case_t::COMPLETED) {
      std::cout << "ERROR : use case PB" << std::endl;
      state = ERROR;
   } else if (d_reservation.count () < u_case->get_duration_max ().count ()) {
      std::cout << "ERROR : too short reservation = " << d_reservation.count () << "    max possible durations = " << u_case->get_duration_max ().count () << std::endl;
      state = ERROR;
   } else {
      state    = READY;
      use_case = u_case;
      stop_requested.store (false);
   }
}

session_t::state_t session_t::get_state () {
   return state;
}

session_t::state_t session_t::start () {
   use_case_t::completion_t complet = use_case_t::ERROR;
   if (state == READY) {
      if (t_begin > std::chrono::system_clock::now ()) {
         std::cout << "Waiting for (" << time_point_str (t_begin) << ") to start ..." << std::endl << std::endl;
         std::this_thread::sleep_until (t_begin);  // todo interruptible
      }
      int n_iter = 0;
      state = RUNNING;
      if (not stop_requested.load ()) {
         std::cout << "Initialization    " << std::flush;
         complet = stage_perform (session_t::INIT);
         std::cout << use_case_t::completion_str (complet) << std::endl;
      }
      if (not stop_requested.load () and complet == use_case_t::COMPLETED) {
         n_iter++;
         std::cout << "Work iteration " << n_iter << "  " << std::flush;
         complet = stage_perform (session_t::WITER);
         std::cout << use_case_t::completion_str (complet) << std::endl;
      }
      while (not stop_requested.load () and complet == use_case_t::TO_BE_CONTINUED) {
         n_iter++;
         std::cout << "Work iteration " << n_iter << "  " << std::flush;
         complet = stage_perform (session_t::WITER);
         std::cout << use_case_t::completion_str (complet) << std::endl;
      }
      if (not stop_requested.load () and complet == use_case_t::COMPLETED) {
         std::cout << "Finish            " << std::flush;
         complet = stage_perform (session_t::FINISH);
         std::cout << use_case_t::completion_str (complet) << std::endl;
         state = COMPLETED;
      } else {
         if (stop_requested.load ())         state = STOPPED;
         if (complet == use_case_t::TIMEOUT) state = KILLED;
      }
   }
   return state;
}

void session_t::stop () {
   stop_requested.store (true);
}


use_case_t::completion_t session_t::stage_perform (stage_typ typ) {
   use_case_t::completion_t                complet;
   std::thread                             stage_thread;
   std::promise <use_case_t::completion_t> stage_promise;
   std::future  <use_case_t::completion_t> stage_future = stage_promise.get_future ();
   switch (typ) {
      case session_t::INIT:
         stage_thread = std::thread (&session_t::init_stage,     this, &stage_promise);
         break;
      case session_t::WITER:
         stage_thread = std::thread (&session_t::work_iteration, this, &stage_promise);
         break;
      case session_t::FINISH:
         stage_thread = std::thread (&session_t::finish_stage,   this, &stage_promise);
   }
   if (stage_future.wait_until (t_end) == std::future_status::timeout) {
      int       res;
      pthread_t handle = stage_thread.native_handle ();
      res              = pthread_setcanceltype      (PTHREAD_CANCEL_ENABLE,       NULL);
      res              = pthread_setcancelstate     (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
      res              = pthread_cancel             (handle);
      complet          = use_case_t::TIMEOUT;
      stage_thread.detach ();
   } else {
      complet = stage_future.get ();
      stage_thread.join          ();
   }
   return complet;
}

void session_t::init_stage (std::promise <use_case_t::completion_t> * complet) {
   complet->set_value (use_case->init_stage ());
}

void session_t::work_iteration (std::promise <use_case_t::completion_t> * complet) {
   complet->set_value (use_case->work_iteration ());
}

void session_t::finish_stage (std::promise <use_case_t::completion_t> * complet) {
   complet->set_value (use_case->finish_stage ());
}

