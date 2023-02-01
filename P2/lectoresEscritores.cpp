#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

mutex mtx;

const int
    numLectores = 3, //Número de lectores
    numEscritores = 3; //Número de escritores

// *****************************************************************************
// clase para monitor Lec_Esc

class Lec_Esc : public HoareMonitor
{
private:
    //variables permanentes
bool
    escrib;                //true si un escritor está escribiendo
unsigned int
    n_lec;                 // número de lectores leyendo en un momento dado

CondVar                    // colas condición:
    lectura,               // cola donde esperan los lectores cuando ya hay un escritor escribiendo
    escritura;             // cola donde esperan los escritores cuando ya hay un escritor escribiendo

public:                    // constructor y métodos públicos
    Lec_Esc();
    void ini_lectura();
    void fin_lectura();
    void ini_escritura();
    void fin_escritura();
} ;
// -----------------------------------------------------------------------------

//IMPORTANTE: El código de escritura no puede ejecutarse concurrentemente
//con ninguna otra escritura ni lectura.

Lec_Esc::Lec_Esc() {
    n_lec = 0;
    escrib = false;
    lectura = newCondVar();
    escritura = newCondVar();
}

void Lec_Esc::ini_lectura() {
    //Si hay un escritor, los lectores esperan
    //IMPORTANTE: Si un escritor está escribiendo, el lector NO puede leer
    if (escrib)
        lectura.wait();

    //Sumamos un lector
    n_lec++;

    //Hacemos un signal para liberar los posibles lectores bloqueados
    lectura.signal();
}

void Lec_Esc::fin_lectura() {
    //Lectura finalizada, quitamos un lector
    n_lec--;

    //Si nos quedamos sin lectores, liberamos (si lo hubiera) a un escritor
    if (n_lec == 0)
        escritura.signal();
}

void Lec_Esc::ini_escritura() {
    //Si hay algún lector o un escritor, el nuevo escritor espera
    //IMPORTANTE: Si un lector está leyendo, el escritor NO puede escribir
    //IMPORTANTE: Si un escritor está escribiendo, NO puede haber otro escritor
    if (n_lec > 0 || escrib)
        escritura.wait();

    //Indicamos que hay un escritor escribiendo
    escrib = true;
}

void Lec_Esc::fin_escritura() {
    //Indicamos que se ha acabado la escritura
    escrib = false;

    //Si la cola de lectura no está vacía (hay lectores esperando), liberamos un lector
    if (!lectura.empty())
        lectura.signal();
    else //En caso contrario, liberamos a un escritor
        escritura.signal();

    //Aclaración: Esto está implementado para darle prioridad a los lectores.
    //Si queremos a la inversa, ponemos el bloque condicional superior al
    //revés
}

//----------------------------------------------------------------------
// Función que simula la acción de leer, como un retardo aleatorio
// de la hebra
void leer(int numHebra){
    //Retardo aleatorio
    chrono::milliseconds duracion(aleatorio<20,200>());

    mtx.lock();
    cout << "El lector " << numHebra << " comienza a leer ("
         << duracion.count() << " milisegundos)" << endl << flush;
    mtx.unlock();

    this_thread::sleep_for(duracion);

    mtx.lock();
    cout << "El lector " << numHebra << " ha terminado de leer "
         << endl << flush;
    mtx.unlock();
}

//----------------------------------------------------------------------
// Función que simula la acción de escribir, como un retardo aleatorio
// de la hebra
void escribir(int numHebra){
    //Retardo aleatorio
    chrono::milliseconds duracion(aleatorio<20,200>());

    mtx.lock();
    cout << "El escritor " << numHebra << " comienza a escribir ("
         << duracion.count() << " milisegundos)." << endl << flush;
    mtx.unlock();

    this_thread::sleep_for(duracion);

    mtx.lock();
    cout << "El escritor " << numHebra << " ha terminado de escribir."
         << endl << flush;
    mtx.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra lectora
void funcion_hebra_lectora( MRef<Lec_Esc>  monitor, int numHebra)
{
    while(true){
        chrono::milliseconds  duracion (aleatorio<20, 200>());
        this_thread::sleep_for(duracion);

        monitor->ini_lectura();
        leer(numHebra);
        monitor->fin_lectura();
    }
}
//----------------------------------------------------------------------
// función que ejecuta la hebra escritora
void  funcion_hebra_escritora(MRef<Lec_Esc>  monitor, int numHebra )
{
   while( true )
   {
       chrono::milliseconds  duracion (aleatorio<20, 200>());
       this_thread::sleep_for(duracion);

       monitor->ini_escritura();
       escribir(numHebra);
       monitor->fin_escritura();
   }
}

//----------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------------------" << endl
         << "Problema de los lectores y escritores (Monitor SU Lec_Esc). " << endl
         << "--------------------------------------------------------------------" << endl
         << flush ;

    // crear monitor ('monitor' es una referencia al mismo, de tipo MRef<...>)
    MRef<Lec_Esc> monitor = Create<Lec_Esc>() ;

    //Crear las hebras
    thread
        hebra_escritora[numEscritores],
        hebra_lectora[numLectores];

    //Recorremos los arrays lanzando en cada posición las hebras
    for(int i=0; i < numEscritores; i++){
        hebra_escritora[i] = thread(funcion_hebra_escritora, monitor, i);
    }

    for(int i=0; i < numLectores; i++){
        hebra_lectora[i] = thread(funcion_hebra_lectora, monitor, i);
    }

    //Esperar a que terminen las hebras
    for (int i=0; i < numEscritores; i++){
        hebra_escritora[i].join();
    }
    for (int i=0; i < numLectores; i++){
        hebra_lectora[i].join();
    }

    return 0;

}
