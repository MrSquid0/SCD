#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 50 ,   // número de items
   tam_vec   = 10 ,   // tamaño del buffer
   tam_hebras = 3;
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

Semaphore
    libres = tam_vec,
    ocupadas = 0;

int
    vec_intermedio[tam_vec], //Vector intermedio (buffer) de tamaño tam_vec
    primera_libre = 0, //Variable para reflejar el estado de ocupación del vector (LIFO) que va desde 0 hasta tam_vec-1
    primera_ocupada = 0; //Variable para FIFO

mutex
    escribir,
    cerrojo;

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   escribir.lock();
   cout << "producido: " << dato_producido << endl << flush ;
   escribir.unlock();
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   escribir.lock();
   cout << "                  consumido: " << dato << endl ;
   escribir.unlock();
}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora( int num_hebra )
{
   int items_por_hebra = num_items/tam_hebras;

    if(num_hebra == tam_hebras-1){
        items_por_hebra = num_items-(items_por_hebra*(tam_hebras-1));
    }
   for( unsigned i = 0 ; i < items_por_hebra ; i++ )
   {
      //Solución válida para FIFO y LIFO
      int dato = producir_dato() ;
      libres.sem_wait();
      //insertar 'dato' en vector intermedio
      cerrojo.lock();
      vec_intermedio[primera_libre] = dato;
      primera_libre = (primera_libre+1)%tam_vec;
      cerrojo.unlock();
      ocupadas.sem_signal();
   }
}



//----------------------------------------------------------------------

void funcion_hebra_consumidora( int num_hebra )
{
    int items_por_hebra = num_items/tam_hebras;

    if(num_hebra==tam_hebras-1)
        items_por_hebra = num_items-(items_por_hebra*(tam_hebras-1));

   for( unsigned i = 0 ; i < items_por_hebra ; i++ )
   {
      //Versión LIFO
      int dato ;
      ocupadas.sem_wait();

      cerrojo.lock();
      //extraer 'dato' del vector intermedio
      dato = vec_intermedio[primera_libre-1];
      primera_libre = (primera_libre-1)%tam_vec;
      cerrojo.unlock();

      libres.sem_signal();
      consumir_dato( dato ) ;

   /* Versión FIFO
       int dato ;
       sem_wait(ocupadas);
       cerrojo.lock();
       dato = vec_intermedio[primera_ocupada];
       primera_ocupada = (primera_ocupada+1) % tam_vec;
       cerrojo.unlock();
       sem_signal(libres);
       consumir_dato( dato );
       */
    }
}



//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO o FIFO ?)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora[tam_hebras],
          hebra_consumidora[tam_hebras];

    for(int i=0; i<tam_hebras; i++){
        hebra_productora[i] = thread(funcion_hebra_productora,i);
    }
    for(int i=0; i<tam_hebras; i++){
        hebra_consumidora[i] = thread(funcion_hebra_consumidora,i);
    }
    for(int i=0; i<tam_hebras; i++){
        hebra_productora[i].join();
    }
    for(int i=0; i<tam_hebras; i++){
        hebra_consumidora[i].join();
    }

   test_contadores();
}