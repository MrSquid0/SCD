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
   tam_vec   = 10 ;   // tamaño del buffer
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

Semaphore
    libres = tam_vec,
    ocupadas = 0;

int
    vec_intermedio[tam_vec], //Vector intermedio (buffer) de tamaño tam_vec
    primera_libre = 0; //Variable para reflejar el estado de ocupación del vector (LIFO) que va desde 0 hasta tam_vec-1
    //primera_ocupada = 0; //Variable para FIFO (comentada ya que se usó la versión LIFO)

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

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

void  funcion_hebra_productora(  )
{
    //Solución válida para FIFO y LIFO
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      libres.sem_wait();
      //insertar 'dato' en vector intermedio
      vec_intermedio[primera_libre] = dato;
      primera_libre = (primera_libre+1)%tam_vec;
      ocupadas.sem_signal();
   }
}



//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
       /* Versión FIFO
        *
      int dato ;
      ocupadas.sem_wait();
      //extraer 'dato' del vector intermedio
      dato = vec_intermedio[primera_ocupada];
      primera_ocupada = (primera_ocupada+1)%tam_vec;
      libres.sem_signal();
      consumir_dato( dato ) ;
        *
        */

      //Versión LIFO
      int dato ;
      ocupadas.sem_wait();
      //extraer 'dato' del vector intermedio
      primera_libre--;
      dato = vec_intermedio[primera_libre];
      libres.sem_signal();
      consumir_dato( dato ) ;
    }
}

//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO o FIFO ?)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   test_contadores();
}
