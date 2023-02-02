#include <iostream>
#include <cassert>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned
   num_items = 40 ,   // número de items
   tam_vec   = 10 ,   // tamaño del buffer
   numProductores = 5, //debe ser divisible a num_items
   numConsumidores = 8, //debe ser divisible a num_items
   itemsPorHebraProductora = num_items / numProductores,
   itemsPorHebraConsumidora = num_items / numConsumidores;

unsigned
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   primera_libre = 0, //Celda a insertar en el buffer (LIFO y FIFO)
   primera_ocupada = 0, //Celda a extraer del buffer (FIFO)
   buffer[tam_vec], //Buffer intermedio por donde se pasan los datos
   contadorItemsProductor[numProductores]{0}; //Contador de número de items producidos por cada hebra productora

Semaphore
    libres = tam_vec, //Indica las celdas libres del buffer
    ocupadas = 0,     //Indica las celdas ocupadas del buffer
    prod = 1,         //Funciona como un mutex de escritura
    cons = 1;         //Funciona como un mutex de lectura

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------
unsigned producir_dato(unsigned numHebraProductora)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   //Asignamos a cada hebra productora la producción consecutiva de items
   const unsigned dato_producido = (numHebraProductora * itemsPorHebraProductora) + contadorItemsProductor[numHebraProductora];
   contadorItemsProductor[numHebraProductora]++;
   cont_prod[dato_producido] ++ ;
   cout << "Dato " << dato_producido << " producido por hebra " << numHebraProductora << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato , unsigned numHebraConsumidora)
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                                 Dato " << dato
   << " consumido por hebra " << numHebraConsumidora << endl ;

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

void  funcion_hebra_productora( int numHebra )
{
    //Solución válida para FIFO y LIFO
   for(unsigned i = 0 ; i < itemsPorHebraProductora ; i++ )
   {
      unsigned dato = producir_dato(numHebra);
      sem_wait(libres);
      sem_wait(prod); //prod.lock();
      buffer[primera_libre] = dato;
      primera_libre = (primera_libre+1) % tam_vec;
      sem_signal(prod); //prod.unlock();
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( int numHebra )
{
   //Solución LIFO
   for( unsigned i = 0 ; i < itemsPorHebraConsumidora ; i++ )
   {
      int dato;
      sem_wait(ocupadas);
      sem_wait(cons); //cons.lock();
      primera_libre--;
      dato = buffer[primera_libre];
      sem_signal(cons); //cons.unlock();
      sem_signal(libres);
      consumir_dato( dato , numHebra);
    }
}

void funcion_hebra_consumidoraFIFO( int numHebra )
{
    //Solución FIFO
    for( unsigned i = 0 ; i < itemsPorHebraConsumidora ; i++ )
    {
        int dato;
        sem_wait(ocupadas);
        sem_wait(cons); // cons.lock();
        dato = buffer[primera_ocupada];
        primera_ocupada = (primera_ocupada+1) % tam_vec;
        sem_signal(cons); // cons.unlock();
        sem_signal(libres);
        consumir_dato( dato, numHebra);
    }
}
//----------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------------------" << endl
         << "Problema del productor-consumidor múltiples (Monitor SU, buffer LIFO). " << endl
         << "Productores: " << numProductores << "   Consumidores: " << numConsumidores <<
         "       Nº de items: " << num_items << endl
         << "--------------------------------------------------------------------" << endl
         << flush ;

   thread hebra_productora [numProductores],
          hebra_consumidora [numConsumidores];

   for (int i=0; i<numProductores; i++){
       hebra_productora[i] = thread(funcion_hebra_productora, i);
   }

    for (int i=0; i<numConsumidores; i++){
        hebra_consumidora[i] = thread(funcion_hebra_consumidora, i);
    }

    for (int i=0; i<numProductores; i++){
        hebra_productora[i].join();
    }

    for (int i=0; i<numConsumidores; i++){
        hebra_consumidora[i].join();
    }
   test_contadores();
}
