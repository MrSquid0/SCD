// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2-mu-LIFO.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con múltiples productores y consumidores)
//
// Opción LIFO
//
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
        numProductores        = 4 , //debe ser divisible a num_items
        numConsumidores       = 5 , //debe ser divisible a num_items
        id_productor_min      = 0 ,
        id_productor_max      = numProductores - 1 ,
        id_buffer             = numProductores ,
        id_consumidor_min     = numProductores+1 ,
        id_consumidor_max     = numProductores + numConsumidores ,
        num_procesos_esperado = numProductores + numConsumidores + 1 ,
        num_items             = 20,
        tam_vector            = 10, //tamaño del vector
        itemsPorProductor     = num_items / numProductores,
        itemsPorConsumidor    = num_items / numConsumidores,
        id_productor = 0,
        id_consumidor = 1;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}
// ---------------------------------------------------------------------
// producir produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir(int numOrdenProductor)
{
   static int contador = numOrdenProductor * itemsPorProductor;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++ ;
   cout << "Productor " << numOrdenProductor << " ha producido valor " << contador << endl << flush;
   return contador ;
}
// ---------------------------------------------------------------------

void funcion_productor(int numProductor)
{
   for (unsigned int i= 0 ; i < itemsPorProductor ; i++ )
   {
      // producir valor
      int valor_prod = producir(numProductor);
      // enviar valor
      cout << "Productor " << numProductor << " va a enviar valor " << valor_prod
           << endl << flush;
      MPI_Ssend(&valor_prod, 1, MPI_INT, id_buffer, id_productor, MPI_COMM_WORLD );
   }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons, int numConsumidor)
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor " << numConsumidor << " ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor(int numConsumidor)
{
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for(unsigned int i=0 ; i < itemsPorConsumidor; i++ )
   {
      MPI_Ssend(&peticion, 1, MPI_INT, id_buffer, id_consumidor, MPI_COMM_WORLD);
      MPI_Recv (&valor_rec, 1, MPI_INT, id_buffer, id_consumidor, MPI_COMM_WORLD, &estado );
      cout << "Consumidor " << numConsumidor << " ha recibido valor " << valor_rec
           << endl << flush ;
      consumir(valor_rec , numConsumidor);
   }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre ( == número de celdas ocupadas)
              etiq_aceptable ;    // identificador de emisor aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido

   for( unsigned int i=0 ; i < num_items*2 ; i++ )
   {
      // 1. determinar si puede enviar solo prod., solo cons, o todos

      if ( primera_libre == 0 )               // si buffer vacío
         etiq_aceptable = id_productor ;       // recibir envíos de solo productores
      else if ( primera_libre == tam_vector ) // si buffer lleno
         etiq_aceptable = id_consumidor ;      // recibir envíos de solo consumidores
      else                                          // si no vacío ni lleno
         etiq_aceptable = MPI_ANY_TAG ;     // recibir envíos de cualquiera

      // 2. recibir un mensaje del emisor o emisores aceptables

      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable,
                MPI_COMM_WORLD, &estado );

      // 3. procesar el mensaje recibido

      switch( estado.MPI_TAG ) // leer emisor del mensaje en metadatos
      {
         case id_productor: // si ha sido el productor: insertar en buffer
            buffer[primera_libre] = valor ;
            primera_libre++;
            cout << "Buffer ha recibido valor " << valor << endl ;
            break;

         case id_consumidor: // si ha sido el consumidor: extraer y enviarle
            primera_libre--;
            valor = buffer[primera_libre] ;
            cout << "Buffer va a enviar valor " << valor << endl ;
            MPI_Ssend(&valor, 1, MPI_INT, estado.MPI_SOURCE,
                      id_consumidor, MPI_COMM_WORLD);
            break;
      }
   }
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;

   //Inicializar MPI, leer identificadores de procesos y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual )
   {
      // ejecutar la operación apropiada a 'id_propio'
      if (id_propio >= id_productor_min && id_propio <= id_productor_max )
         funcion_productor(id_propio);
      else if ( id_propio == id_buffer )
         funcion_buffer();
      else
         funcion_consumidor(id_propio - id_consumidor_min);
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
