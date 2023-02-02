// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 2. Casos prácticos de monitores en C++11.
//
// Archivo: prodcons_mu_FIFO.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con productores y consumidores múltiples.
// Opción FIFO
//
// Historial:
// Creado el 30 Sept de 2022. (adaptado de prodcons2_su.cpp)
// 20 oct 22 --> paso este archivo de FIFO a LIFO, para que se corresponda con lo que dicen las transparencias
// -----------------------------------------------------------------------------------


#include <iostream>
#include <cassert>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int
   num_items = 15 ;   // número de items a producir/consumir

const unsigned
    numProductores = 5,   //debe ser divisible a num_items
    numConsumidores = 3,  //debe ser divisible a num_items
    itemsPorHebraProductora = num_items / numProductores,
    itemsPorHebraConsumidora = num_items / numConsumidores;
int
   siguiente_dato = 0 ; // siguiente valor a devolver en 'producir_dato'

constexpr int
   min_ms    = 5,     // tiempo mínimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for


mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items] = {0}, // contadores de verificación: producidos
   cont_cons[num_items] = {0}, // contadores de verificación: consumidos
   contadorItemsProductor[numProductores]{0}; //Contador de número de items producidos por cada hebra productora

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato( unsigned numHebraProductora )
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   //Asignamos a cada hebra productora la producción consecutiva de items
   const unsigned dato_producido = (numHebraProductora * itemsPorHebraProductora) + contadorItemsProductor[numHebraProductora];
   contadorItemsProductor[numHebraProductora]++;
   mtx.lock();
   cout << "Dato " << dato_producido << " producido por hebra " << numHebraProductora << endl << flush ;
   mtx.unlock();
   cont_prod[dato_producido]++ ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned valor_consumir , unsigned numHebraConsumidora)
{
   if ( num_items <= valor_consumir )
   {
      cout << " valor a consumir === " << valor_consumir << ", num_items == " << num_items << endl ;
      assert( valor_consumir < num_items );
   }
   cont_cons[valor_consumir] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
    cout << "                                 Dato " << valor_consumir
         << " consumido por hebra " << numHebraConsumidora << endl ;   mtx.unlock();
}
//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

class ProdConsMu : public HoareMonitor
{
 private:
 static const int           // constantes ('static', ya que no dependen de la instancia)
   num_celdas_total = 10;   //   núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//   buffer de tamaño fijo, con los datos
   primera_libre,           //   índice de celda de la próxima inserción
   primera_ocupada,         // índice de la primera celda ocupada
   celdasOcupadas;          // Número de celdas ocupadas del buffer

 CondVar                    // colas condición:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor (n<num_celdas_total)

 public:                       // constructor y métodos públicos
   ProdConsMu() ;
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdConsMu::ProdConsMu(  )
{
   primera_libre = 0 ;
   primera_ocupada = 0;
   celdasOcupadas = 0;
   ocupadas      = newCondVar();
   libres        = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el productor para insertar un dato
void ProdConsMu::escribir(int valor )
{
    // esperar bloqueado hasta que primera_libre < num_celdas_total
    if (celdasOcupadas == num_celdas_total )
        libres.wait();

    //cout << "leer: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
    assert(celdasOcupadas < num_celdas_total );

    // hacer la operación de inserción, actualizando estado del monitor
    buffer[primera_libre] = valor ;
    primera_libre = (primera_libre+1)%num_celdas_total ;
    celdasOcupadas++;

    // señalar al consumidor que ya hay una celda ocupada (por si está esperando)
    ocupadas.signal();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato
int ProdConsMu::leer(  )
{
   // esperar bloqueado hasta que 0 < primera_libre
   if (celdasOcupadas == 0 )
      ocupadas.wait();

   //cout << "leer: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   assert(0 < celdasOcupadas  );

   // hacer la operación de lectura, actualizando estado del monitor
   celdasOcupadas-- ;
   const int valor = buffer[primera_ocupada] ;
   primera_ocupada = (primera_ocupada+1)%num_celdas_total;

   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora(MRef<ProdConsMu> monitor , unsigned numHebra )
{

   for( unsigned i = 0 ; i < itemsPorHebraProductora ; i++ )
   {
      int valor = producir_dato( numHebra ) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora(MRef<ProdConsMu>  monitor , unsigned numHebra)
{

   for( unsigned i = 0 ; i < itemsPorHebraConsumidora ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor , numHebra) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------------------" << endl
        << "Problema del productor-consumidor múltiples (Monitor SU, buffer FIFO). " << endl
        << "Productores: " << numProductores << "   Consumidores: " << numConsumidores <<
        "       Nº de items: " << num_items << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<ProdConsMu> monitor = Create<ProdConsMu>() ;

   // crear y lanzar las hebras
   thread hebra_prod[numProductores],
          hebra_cons[numConsumidores];

   for (int i=0; i<numProductores; i++){
       hebra_prod[i] = thread(funcion_hebra_productora, monitor, i);
   }

   for (int i=0; i<numConsumidores; i++){
       hebra_cons[i] = thread(funcion_hebra_consumidora, monitor, i);
   }

   // esperar a que terminen las hebras
   for (int i=0; i<numProductores; i++){
       hebra_prod[i].join();
   }

    for (int i=0; i<numConsumidores; i++){
        hebra_cons[i].join();
    }

   test_contadores() ;
}
