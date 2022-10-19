#include <iostream>
#include <thread>

using namespace std;

void funcion_hebra_1( ) // función que va a ejecutar la hebra primera
{ for( unsigned long i = 0 ; i < 5000 ; i++ )
        cout << "hebra 1, i == " << i << endl ;
}
void funcion_hebra_2( ) // función que va a ejecutar la hebra segunda
{ for( unsigned long i = 0 ; i < 5000 ; i++ )
        cout << "hebra 2, i == " << i << endl ;
}
int main()
{
    thread hebra1( funcion_hebra_1 ), // crear hebra1 ejecutando funcion_hebra_1
    hebra2( funcion_hebra_2 ); // crear hebra2 ejecutando funcion_hebra_2
// ... finalizacion ....
    hebra1.join();
    hebra2.join();
}