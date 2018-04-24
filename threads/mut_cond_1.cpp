
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std::placeholders;

class Application {

   int                     initial_stock;
   int                     nb_clients;
   int                     stock;
   bool                    stop;
   std::mutex              stock_lock;
   std::condition_variable client_cond;
   std::condition_variable store_cond;

public:

   Application (int ini_stock, int n_clients) {
      initial_stock = ini_stock;
      nb_clients    = n_clients;
      stock         = initial_stock;
      stop          = false;
   }

   void Stop () {
      stop = true;
      store_cond.notify_one ();
   }

   bool Is_Stock_Ok (int val) {
      return val <= stock;
   }

   bool Is_Stock_Not_Full () {
      return stock < 10 or stop;
   }

   /* Fonction pour tirer un nombre au sort entre 0 et max. */
   int get_random (int max) {
      double val;
      val = (double) max * rand ();
      val = val / (RAND_MAX + 1.0);
      return (1 + (int) val);
   }

   /* Fonction pour le thread du magasin. */
   void fn_store () {
      while (1) {
         {
            std::unique_lock<std::mutex> mlock (stock_lock);
            store_cond.wait (mlock, std::bind (&Application::Is_Stock_Not_Full, this));
            if (stop) return;
            stock = initial_stock;
            std::cout << "Remplissage du stock de " << stock << "articles !" << std::endl;
            client_cond.notify_all ();
         }
      }
      std::cout << "Store is closing !!!!" << std::endl;
   }


   /* Fonction pour les threads des clients. */
   void fn_clients (int nb) {
      int  n;
      for (n=0; n<10; n++) {
         int val = get_random (6);
         {
            std::unique_lock<std::mutex> mlock (stock_lock);
            std::cout << "Client " << nb << " veut  " << val << std::endl;
            if (not Is_Stock_Ok (val)) {
               store_cond.notify_one ();
               client_cond.wait (mlock, std::bind (&Application::Is_Stock_Ok, this, val));
            }
            stock = stock - val;
            std::cout << "Client " << nb << " prend " << val << " du stock, reste " << stock << " en stock ! n=" << n << std::endl;
            //client_cond.notify_one ();
         }
         std::this_thread::yield ();
      }
      std::cout <<  "!!!!!!!!!!!!!!! CLIENT " << nb << " A FINI !!!!!!!!!!!!" << std::endl;
   }

};


int main () {

   time_t t;
   int stock_base = 20;
   int nb_clients = 10;

   Application app (stock_base, nb_clients);

   std::thread store;
   std::thread client [nb_clients];

   /* init random generator */
   srand ((unsigned) time (&t));

   /* Creation des threads. */
   std::cout << "Creation du thread du magasin !" << std::endl;
   store = std::thread (&Application::fn_store, &app);

   /* Creation des threads des clients si celui du magasinn a reussi. */
   std::cout << "Creation des threads clients !" << std::endl;
   for (int i = 0; i < nb_clients; i++) {
      client [i] = std::thread (&Application::fn_clients, &app, i);
   }

   std::cout << ".................  WORKING  ................." << std::endl;
   /* Attente de la fin des threads. */
   for (int i = 0; i < nb_clients; i++) {
      std::cout << "====> waiting for client " << i << std::endl;
      client [i].join ();
   }

   std::cout << "====> waiting for store to finish " << std::endl;
   app.Stop ();
   store.join ();
   return EXIT_SUCCESS;
}

