#include <pthread.h>
#include <csignal>

#include <iostream>
#include <thread>
#include <future>

#include "time_tools.h"

//////////////////////
//
//
    int TST_SEC = 17;
//
//
//////////////////////

void fn_test (std::promise<int> * promiz) {
   for (int i=0; i<TST_SEC; i++) {
      std::cout << "====  fn_test  ===" << std::endl;
      std::this_thread::sleep_for (std::chrono::seconds (1));
   }
   std::cout << "Choz promiz ..." << std::endl;
   promiz->set_value (35);
}


int main () {

   auto t0 = std::chrono::system_clock::now ();    //  begin of prog

   std::promise <int> promiz;
   std::future  <int> futfut = promiz.get_future ();
   std::thread th = std::thread (fn_test, &promiz);

   if (futfut.wait_for (std::chrono::seconds (5)) == std::future_status::timeout) {
      std::cout << "Thread time out !!!" << std::endl;
      int  res;
      pthread_t handle = th.native_handle       ();
      res              = pthread_setcanceltype  (PTHREAD_CANCEL_ENABLE, NULL);
      res              = pthread_setcancelstate (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
      res              = pthread_cancel         (handle);
      std::cout << "Thread canceled !!!" << std::endl;
      th.detach ();
   } else {
      std::cout << "Thread OK"   << std::endl;
      std::cout << futfut.get () << std::endl;
      th.join ();
   }

   auto tm  = std::chrono::system_clock::now ();    //  mid prog

   std::this_thread::sleep_for (std::chrono::seconds (5));

   auto tf  = std::chrono::system_clock::now ();    //  end of prog

   std::cout << "begin  = " << time_point_str (t0)  << std::endl;
   std::cout << "midway = " << time_point_str (tm)  << std::endl;
   std::cout << "end    = " << time_point_str (tf)  << std::endl;

   return EXIT_SUCCESS;
}
