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

//Retardo aleatorio
chrono::milliseconds duracion(aleatorio<20,200>());

//Funciones de prints de lectura y escritura
void imprimeLectura(int num_hebra){
    mtx.lock();
    cout << "El lector " << num_hebra << " comienza a leer ("
         << duracion.count() << " milisegundos)" << endl << flush;
    mtx.unlock();

    this_thread::sleep_for(duracion);

    mtx.lock();
    cout << "El lector " << num_hebra << " ha terminado de leer "
         << endl << flush;
    mtx.unlock();
}

void imprimeEscritura(int num_hebra){
    mtx.lock();
    cout << "El escritor " << num_hebra << " comienza a escribir ("
         << duracion.count() << " milisegundos)"<< endl << flush;
    mtx.unlock();

    this_thread::sleep_for(duracion);

    mtx.lock();
    cout << "El escritor " << num_hebra << " ha terminado de escribir "
         << endl << flush;
    mtx.unlock();
}

// *****************************************************************************
// clase para monitor Lec_Esc

class Lec_Esc : public HoareMonitor
{
private:
    //variables permanentes
bool
    escrib;                //true si un escritor está escribiendo
unsigned int
    n_lec,                 // número de lectores leyendo en un momento dado
    contador;

CondVar                    // colas condicion:
    lectura,               // cola donde esperan los lectores cuando ya hay un escritor escribiendo
    escritura;             // cola donde esperan los escritores cuando ya hay un escritor escribiendo

public:                    // constructor y métodos públicos
    Lec_Esc();             // constructor
    void ini_lectura();
    void fin_lectura();
    void ini_escritura();
    void fin_escritura();
} ;
// -----------------------------------------------------------------------------

Lec_Esc::Lec_Esc() {
    n_lec = 0;
    escrib = false;
    lectura = newCondVar();
    escritura = newCondVar();
}

void Lec_Esc::ini_lectura() {
    //Si hay un escritor, esperamos
    if (escrib || (contador%5 == 0))
        lectura.wait();

    //Comprobamos que el parámetro es correcto
    assert (escrib == false);

    contador++;
    //Sumamos un lector
    n_lec++;

    //Hacemos un signal para liberar las posibles hebras bloqueadas
    lectura.signal();
}

void Lec_Esc::fin_lectura() {
    //Lectura finalizada, quitamos un lector
    n_lec--;

    //Si nos quedamos sin lectores, liberamos al escritor (ya que nadie leerá)
    if (n_lec == 0)
        escritura.signal();
}

void Lec_Esc::ini_escritura() {
    //Si hay algún lector o si se está escribiendo, esperamos
    if (n_lec > 0 || escrib)
        escritura.wait();

    //Si empezamos a escribir, establecemos la variable a true para indicarlo
    escrib = true;
}

void Lec_Esc::fin_escritura() {
    //Indicamos que se ha acabado la escritura
    escrib = false;

    //Si la cola de lectura está vacía, el lector espera
    if (!lectura.empty())
        lectura.signal();
    else //En caso contrario, liberamos a un escritor
        escritura.signal();

    //Aclaración: Esto está implementado para darle prioridad a los lectores.
    //Si queremos a la inversa, ponemos el bloque condicional superior al
    //revés
}

//----------------------------------------------------------------------
// función que ejecuta la hebra lectora

void funcion_hebra_lectora( MRef<Lec_Esc>  monitor, int num_hebra)
{
    while(true){
        monitor->ini_lectura();

        imprimeLectura(num_hebra);

        monitor->fin_lectura();
    }
}
//----------------------------------------------------------------------
// función que ejecuta la hebra escritora
void  funcion_hebra_escritora(MRef<Lec_Esc>  monitor, int num_hebra )
{
   while( true )
   {
       monitor->ini_escritura();

       imprimeEscritura(num_hebra);

       monitor->fin_escritura();
   }
}

//----------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------------------" << endl
         << "Problema de los lectores y escritores (Monitor Lec_Esc). " << endl
         << "--------------------------------------------------------------------" << endl
         << flush ;

    // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
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
