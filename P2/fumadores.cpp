#include <iostream>
#include <cassert>
#include <thread>
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// número de fumadores
const int num_fumadores = 3 ;

// *****************************************************************************
// clase para monitor Estanco

class Estanco : public HoareMonitor
{
private:
    int                             //variables permanentes
    mostrador;                 // -1 si está libre / i{0,1,2} si está el ingr. i en el mostrador

    CondVar                        // colas condición:
    estanquero,               // cola donde espera el estanquero si el mostrador está vacío
    fumadores[num_fumadores];  // cola donde esperan los fumadores a recoger el ingrediente i

public:                    // constructor y métodos públicos
    Estanco() ;             // constructor
    void ponerIngrediente (int ingrediente);
    void esperarRecogidaIngrediente();
    void obtenerIngrediente(int ingrediente);
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco() {
    mostrador = -1;
    estanquero   = newCondVar();
    //Inicializar cada posición del array de cola
    for (int i = 0; i<num_fumadores; i++)
        fumadores[i] = newCondVar();
}

void Estanco::ponerIngrediente(int ingrediente) {
    //Imprimimos por consola la información
    cout << "El ingrediente " << ingrediente << " se ha puesto en el mostrador "
         << "para ser recogido." << endl;

    //Ponemos en el mostrador el ingrediente
    mostrador = ingrediente;

    //Indicamos a la cola fumadores que hay un ingrediente disponible
    fumadores[ingrediente].signal();
}

void Estanco::esperarRecogidaIngrediente() {
    //Si el mostrador no está vacío, hacemos esperar en la cola estanquero
    if (mostrador != -1)
        estanquero.wait();
}

void Estanco::obtenerIngrediente(int ingrediente) {
    //Nos aseguramos que los parámetros son correctos
    assert(0 <= ingrediente && ingrediente < num_fumadores);

    //Comprobamos si el mostrador tiene el ingrediente indicado
    if (mostrador != ingrediente)
        fumadores[ingrediente].wait();

    //Imprimimos por consola la información
    cout << "El ingrediente " << ingrediente << " ha sido recogido." << endl;

    //Indicamos que el mostrador vuelve a estar vacío
    mostrador = -1;

    //Indicamos al estanquero que el mostrador se ha quedado vacío
    estanquero.signal();
}

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracionProduciendo(aleatorio<10,100>() );

    //Número de ingrediente a producir (elegido aleatoriamente)
    const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

    // informa de que comienza a producir
    cout << "Estanquero : empieza a producir ingrediente " << num_ingrediente <<
         " (" << duracionProduciendo.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracionProduciendo' milisegundos
    this_thread::sleep_for(duracionProduciendo );

    // informa de que ha terminado de producir
    cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

    return num_ingrediente ;
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatorio de la hebra

void fumar(int num_fumador )
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}
//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco>  monitor )
{
    while(true){
        int ingrediente = producir_ingrediente();
        monitor->ponerIngrediente(ingrediente);
        monitor->esperarRecogidaIngrediente();
    }
}
//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador(MRef<Estanco>  monitor, int num_fumador )
{
   assert(0 <= num_fumador && num_fumador < num_fumadores);
   while( true )
   {
       monitor->obtenerIngrediente(num_fumador);
       fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------------------" << endl
         << "Problema de los fumadores (Monitor SU Estanco). " << endl
         << "--------------------------------------------------------------------" << endl
         << flush ;

    // crear monitor ('monitor' es una referencia al mismo, de tipo MRef<...>)
    MRef<Estanco> monitor = Create<Estanco>() ;

    //Crear y lanzar las hebras
    thread
        hebra_fumador[num_fumadores],
        hebra_estanquero(funcion_hebra_estanquero, monitor);

    for(int i=0; i<num_fumadores; i++){
        hebra_fumador[i] = thread(funcion_hebra_fumador, monitor, i);
    }

    //Esperar a que terminen las hebras
    hebra_estanquero.join();
    for (int i=0; i<num_fumadores; i++){
        hebra_fumador[i].join();
    }
}
