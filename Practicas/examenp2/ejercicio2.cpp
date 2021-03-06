/**************************** /
	Carlos Sánchez Páez
			2A (A2)
		DNI: 25613096-C
/*****************************/

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

//////////////////////////////////////

// He utilizado una implementación mediante un vector de variables condición.
    // Por tanto, en las hebras en lugar de pasar el parámetro 1 ó 2 paso 0 (blanca) ó 1 (color) para un correcto uso de los índices.

const int NUM_TIPO_1=5; //Número de clientes con colada blanca.
const int NUM_TIPO_2=4; //Número de clientes con colada de color.


/********Declaración del monitor*************/

class Lavanderia : public HoareMonitor{

    private:

        static const int N_L_BLANCAS=3; //Número de lavadoras blancas.
        static const int N_L_COLOR=2;   //Número de lavadoras de color.
        CondVar lavadora_tipo[2];       //Colas para cada lavadora

        //Valores para ver la primera lavadora libre de cada tipo. 
       
        int primera_libre_blanca;       
        int primera_libre_color;

        bool todasLlenas(int tipo);     //Función privada que calcula si están en uso todas las lavadoras de un determinado tipo.

       public:

        Lavanderia();       //Constructor

        void llegada(int tipo);

        void finColada(int tipo);
};

/************************************/

/******Espera aleatoria**************/

template< int min, int max > int aleatorio()
{
	static default_random_engine generador( (random_device())() );
	static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
	return distribucion_uniforme( generador );
}

void esperaAleatoria(){
	chrono::milliseconds espera( aleatorio<60, 200>() );
	this_thread::sleep_for( espera );
}

/************************************/


/*****Hebras a ejecutar*************/

void funcion_hebra_cliente_colada_blanca(MRef<Lavanderia> m){
    while(true){
        m->llegada(0);
        esperaAleatoria();
        m->finColada(0);
        esperaAleatoria();
    }
}

void funcion_hebra_cliente_colada_color(MRef<Lavanderia> m){
    while(true){
        m->llegada(1);
        esperaAleatoria();
        m->finColada(1);
        esperaAleatoria();
    }
}

/************************************/


/*********Implementación del monitor***********************/

Lavanderia::Lavanderia(){
    for(int i=0;i<2;i++)
        lavadora_tipo[i]= newCondVar();
    primera_libre_blanca=0;
    primera_libre_color=0;
}

bool Lavanderia::todasLlenas(int tipo){
    bool aDevolver;
    if(tipo == 0)   //Blanca
        aDevolver= primera_libre_blanca == N_L_BLANCAS;
    else            //Color
        aDevolver= primera_libre_color == N_L_COLOR;
    return aDevolver;
}

void Lavanderia::llegada(int tipo){
    if( todasLlenas(tipo) ){
        cout<<"Todas las lavadoras de tipo "<<(tipo == 0 ? "blanco" : "color")<<" estan llenas. Esperamos..."<<endl;
        lavadora_tipo[tipo].wait();
    }
    cout<<"Metemos la ropa de tipo "<<(tipo == 0 ? "blanco" : "color")<<" en la lavadora."<<endl;
    //El ajuste +1 es para evitar salidas como "Hay 0 lavadoras llenas " (cuando sólo llenamos una).
    cout<<"Actualmente hay "<<(tipo == 0 ? primera_libre_blanca+1 : primera_libre_color+1 )<<" lavadoras llenas de tipo "<<(tipo == 0 ? "blanco" : "color")<<endl;
    if( tipo == 0 ){ //Blanca
        primera_libre_blanca++;
    }
    else {   //Color
        primera_libre_color++;
    }      

}

void Lavanderia::finColada(int tipo){
    cout<<"Finaliza una colada de tipo "<<(tipo == 0 ? "blanco" : "color") << "."<<endl;
    //El ajuste -1 es porque todavía no he  decrementado la primera libre.
    cout<<"Actualmente hay "<<(tipo == 0 ? primera_libre_blanca-1 : primera_libre_color-1 )<<" lavadoras llenas de tipo "<<(tipo == 0 ? "blanco" : "color")<<endl;  

    if( tipo == 0 ) {    //Blanca
        primera_libre_blanca--;
        lavadora_tipo[0].signal();
    }else
        primera_libre_color--;
        lavadora_tipo[1].signal();   
}


/**********************************************************/

int main(){

    //Declaración

    thread clientes_colada_blanca [NUM_TIPO_1];
    thread clientes_colada_color  [NUM_TIPO_2];
    MRef<Lavanderia> monitor = Create <Lavanderia> ();

    //Inicializamos hebras
    
    for(int i=0;i<NUM_TIPO_1;i++)
        clientes_colada_blanca [i] = thread ( funcion_hebra_cliente_colada_blanca, monitor);
    for(int j=0;j<NUM_TIPO_2;j++)
        clientes_colada_color [j] = thread ( funcion_hebra_cliente_colada_color, monitor);

    //Hacemos join

    for(int i=0;i<NUM_TIPO_1;i++)
        clientes_colada_blanca[i].join();

    for(int j=0;j<NUM_TIPO_2;j++)
        clientes_colada_color[j].join();

}
