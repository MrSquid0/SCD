// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos.cpp
// Implementación del problema de los filósofos (sin camarero)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,              // número de filósofos 
   num_filo_ten  = 2*num_filosofos, // número de filósofos y tenedores
   num_procesos  = num_filo_ten ;   // número de procesos total (por ahora solo hay filo y ten)


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

void funcion_filosofos( int id )
{
  int id_ten1, //id tenedor 1
      id_ten2, //id tenedor 2
      peticion;

  while ( true )
  {
      if (id < num_filosofos){
          id_ten1 = (id+1) % num_procesos;
          id_ten2 = (id+num_procesos-1) % num_procesos;
      } else {
          id_ten1 = (id+num_procesos-1) % num_procesos;
          id_ten2 = (id+1) % num_procesos;
      }
    cout <<"Filósofo " <<id << " solicita ten. " <<id_ten1 <<endl;
    //solicitar tenedor 1
    MPI_Ssend(&peticion, 1, MPI_INT, id_ten1, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. " <<id_ten2 <<endl;
    //solicitar tenedor 2
    MPI_Ssend(&peticion, 1, MPI_INT, id_ten2, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. " <<id_ten1 <<endl;
    //soltar el tenedor 1
    MPI_Ssend(&peticion, 1, MPI_INT, id_ten1, 0, MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. " <<id_ten2 <<endl;
    //soltar el tenedor 2
    MPI_Ssend(&peticion, 1, MPI_INT, id_ten2, 0, MPI_COMM_WORLD);

    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     //recibir petición de cualquier filósofo
     MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
     id_filosofo = estado.MPI_SOURCE; //guardar en 'id_filosofo' el id. del emisor
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;
     //recibir liberación de filósofo 'id_filosofo' (completar)
      MPI_Recv(&valor, 1, MPI_INT, id_filosofo, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
      cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
