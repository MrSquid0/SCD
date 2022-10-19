#include <iostream>
#include <iomanip>
#include <chrono>
#include <future>
#include <vector>
#include <cmath>

using namespace std;
using namespace std::chrono;

const long numMuestras = 1073741824;
const int numHebras = 4;

//Función a integrar
double f( double x ){
    return 4.0/(1+x*x) ; // f (x) = 4/(1 + x2 )
}

double calcular_integral_secuencial( ){
    double suma = 0.0 ; // inicializar suma
    for(long j = 0 ; j < numMuestras ; j++ ) // para cada j entre 0 y numMuestras − 1:
    { const double xj = double(j+0.5) / numMuestras; // calcular xj
        suma += f( xj ); // añadir f (xj ) a suma
    }
    return suma / numMuestras ; // devolver valor promedio de f
}

double funcion_hebra(long ih)
{
    double suma= 0.0;
    for(unsigned long j=ih; j < numMuestras; j+=numHebras)
        suma += f((j+double(0.5)) / numMuestras);
    return suma;
}

double calcular_integral_concurrente( )
{
    double suma= 0.0; //Inicializamos suma
    future<double> futuros[numHebras]; //Declaramos el future de doubles
    for(int i=0; i<numHebras; i++)
        futuros[i]=async(launch::async, funcion_hebra, i);
    for(int i=0; i<numHebras; i++)
        suma+=futuros[i].get();
    return suma / numMuestras;
}

int main( )
{
    // hacer los cálculos y medir los tiempos:
    time_point<steady_clock> instante_inicio_sec = steady_clock::now();
    const double pi_sec = calcular_integral_secuencial( );
    time_point<steady_clock> instante_final_sec = steady_clock::now();
    double x = sin(0.4567);
    time_point<steady_clock> instante_inicio_conc = steady_clock::now();
    const double pi_conc = calcular_integral_concurrente( );
    time_point<steady_clock> instante_final_conc = steady_clock::now();
    duration<float,milli> tiempo_sec = instante_final_sec - instante_inicio_sec;
    duration<float,milli> tiempo_conc = instante_final_conc - instante_inicio_conc;
    const float porcentaje = 100 * tiempo_conc.count() / tiempo_sec.count();

    const double pi = 3.14159265358979312 ; // valor de π con bastantes decimales

// escribir en cout los resultados:
    cout << "Numero de muestras (numMuestras) : " << numMuestras << endl
         << "Numero de hebras (n) : " << numHebras << endl
         << setprecision(18)
         << "Valor de PI : " << pi << endl
         << "Resultado secuencial : " << pi_sec << endl
         << "Resultado concurrente : " << pi_conc << endl
         << setprecision(5)
         << "Tiempo secuencial : " << tiempo_sec.count() << " milisegundos. " << endl
         << "Tiempo concurrente : " << tiempo_conc.count() << " milisegundos. " << endl
         << setprecision(4)
         << "Porcentaje t.conc/t.sec. : " << porcentaje << "%" << endl;
}