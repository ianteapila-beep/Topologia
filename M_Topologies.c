#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
//Constante para la cantidad de hilos de esta computadora.
#define NUM_HILOS 8
int T = 0;
pthread_mutex_t mutex_impresion = PTHREAD_MUTEX_INITIALIZER;

int topologias = 0;
int no_topologias = 0;

//Estructura para cada hilo del procesador.
typedef struct 
{
    unsigned long long inicio;
    unsigned long long fin;
} ArgumentosHilo;

//Funciones para ingresar los elementos del conjunto.
char elementos[8][6];
int Compara(int);
void Ingresa_Elementos();

//Funcion para los elementos del subconjunto ingresado.
void imp_elemento(int A);

//Verificamos que la union de vecindades de un conjunto A forman al conjunto A.
int  Columnas(int c, int *M);

void *BuscarTopologias(void *arg);
void Reglas();

int main() 
{
    Reglas();
    Ingresa_Elementos();
    
    //PARA SE PUEDA EVALUAR UNICAMENTE ESTA CANDIDAD DE ELEMENTOS ES NECESARIO FORZAR LA REFLEXIVIDAD EN LA MATRIZ.
    unsigned long long total_elementos = 1ULL << ((T * T) - T);

    pthread_t hilos[NUM_HILOS];
    ArgumentosHilo args_hilos[NUM_HILOS];
    
    unsigned long long bloque = total_elementos / NUM_HILOS;

    //Dividimos la cantidad de iteraciones que requiere el programa entre la cantidad de hilos del procesador.
    printf("TOTAL DE MATRICES A EVALUAR: %llu\n", total_elementos);
    if(total_elementos > 1ULL << T*5)
    {
        printf("El proceso podria tardar demasiado tiempo\n");
    }
    printf("Presiona ENTER para continuar\n");
    getchar();

    for (int i = 0; i < NUM_HILOS; i++) 
    {
        args_hilos[i].inicio = i * bloque;
        
        if (i == NUM_HILOS - 1) args_hilos[i].fin = total_elementos; 
        else                    args_hilos[i].fin = (i + 1) * bloque;

        pthread_create(&hilos[i], NULL, BuscarTopologias, (void *)&args_hilos[i]);
    }
    //Esperamos a que todos lo hilos del procesador terminen de ejecutar la tarea asignada.
    for (int i = 0; i < NUM_HILOS; i++)
        pthread_join(hilos[i], NULL);

    printf("Total de subconjuntos que son topologias: %d\n", topologias);
    printf("Total de subconjuntos que NO son topologias: %d\n", no_topologias);

    return 0;
}
void Reglas()
{

    printf("--------------------------------------------------------------------\n");
    printf("ESTE PROGRAMA ESTA BASADO EN MAYOR PARTE DEL ARTICULO \"COMPUTING TOPOLOGIES\" DE L. W. BRINN\n");
    printf("SIN EMBARGO PARA PODER COMPRENDER PORQUE FUNCIONA TENEMOS QUE TOMAR EL LIBRO DE BIRKHOFF TITULADO \"LATTICE THEORY\".\n");
    printf("Resumiendo de manera muy breve, en este ultimo se demuestra que:\n");
    printf("1) Un Conjunto Parcialmente Ordenado (COPO/POSET) tiene relacion biunivoca con espacios topologicos finitos del tipo T_0.\n");
    printf("2) La estructura algebraica que forma es un anillo Isomorfo.\n");
    printf("3) Todo preorden(cuasi-orden) genera una relacion de equivalencia, por lo que tiene las propiedades de un COPO/POSET ");
    printf("siendo la unica diferencia que un cuasi-orden no requiere de antisimetria ni tampoco generar necesariamente un espacio topologico del tipo T_0.\n\n");

    printf("Ahora bien, el articulo de Brinn es el que relaciona lo anterior con algebra de Boole, mencionando que:\n");
    printf("Todo preorden(cuasi-orden) en un conjunto finito puede representarse mediante una matriz booleana que sea reflexiva y transitiva.\n");
    printf("En esta matriz, cada columna representa la 'vecindad abierta minima' (la base) de cada elemento.\n");
    printf("Por lo tanto, al generar todas las uniones posibles (disyunciones a nivel de bits) de estas columnas, se construye exactamente la topologia completa.\n\n");

    printf("Debido a 2) y 3) se pueden reducir las opciones y con ello los calculos necesarios para encontrar topologias ");
    printf("requiriendo evaluar unicamente 2^(T^2 - T) elementos, donde T es la cardinalidad del conjunto ingresado.\n");
    printf("--------------------------------------------------------------------\n\n");

    printf("IMPORTANTE!\n");
    printf("-En teoria este programa puede computar un conjunto de hasta 8 elementos sin perder precision.\n");
    printf("-Puedes ingresar cualquier combinacion de hasta 5 caracteres alfanumericos.\n");
    printf("PRESIONA ENTER PARA CONTINUAR\n");
    getchar();
}

//Funcion para imprimir los elementos del subconjunto generado.
void imp_elemento(int A) 
{
    //A representa la union(suma/disyuncion) de todos los elementos que componen el subconjunto, por lo que si
    // es 0 significa que es el vacio.
    if (A == 0)
        printf("{}");
    else 
    {
        printf("{");
        int primero = 1;
        for (int i = 0; i < T; i++)
            if (A & (1 << i)) //El operador 'a << n' se usa para recorrer n bits a la izquierda desde la posicion de a.
            {
            //Mostramos los elementos donde la interseccion de bits coinciden, por ejemplo:
            /*Si tenemos A = 23 a nivel de bits se representa como 
            A = [0]... [0] [1] [0] [1] [1] [1] = 0*2^n+...0*2⁵+ 2⁴ + 0*2³ + 2² + 2¹ + 2⁰
              =  0+...+ 0+ 16 + 0 + 4 + 2 + 1
            el ciclo comienza con i = 0, por lo que i representa el exponente de 2. 
            */
                if (!primero) printf(","); //Unicamente en el primer caso no imprimimos la coma que separa los elementos.
                printf("%s",elementos[i]);
                primero = 0;
            }
        printf("}");
    }
}

//Funcion para hacer la union de columnas de la matriz.
int Columnas(int c, int *M)
{
    int aux = 0;
    for (int x = 0; x < T; ++x)
        aux |= (M[x * T + c] << x);
    //De acuerdo al libro de Garret cada columna de la matriz representa una vecindad, por ello 
    //si se al hacer la union de vecindades se debe generar el conjunto(subconjunto) al que pertenecen.
    return aux;
}


void *BuscarTopologias(void *arg)
{
    ArgumentosHilo *args = (ArgumentosHilo *)arg;
    
    int *M = (int *)malloc(T * T * sizeof(int));

    //i va a ser la representacion binaria de cada elemento.
    for (unsigned long long i = args->inicio; i < args->fin; i++) 
    {    
        /*Rellenamos la matriz en base a i, es decir:
        Como todos los elementos estan asociados a un unico numero, al hacer la comparacion 
        bit a bit con una potencia de 2^e obtenemos 0 o 1 dependiendo en que 2^e coinciden los bits, por ejemplo:
        Si ingresas el conjunto A = {paco, luis}, tenemos que |A| = 2,  tendriamos que evaluar
                2^(2²-2) = 4 matrices de 2x2 las cuales son:
                i = 0        i = 1      i = 2       i = 3
                [1][0]       [1][1]     [1][0]      [1][1]
                [0][1]       [0][1]     [1][1]      [1][1]
        */
        for (int x = 0, e = 0; x < T; ++x)
        for (int y = 0; y < T; ++y) 
        {
            if (x != y) 
            {
                M[x * T + y] = (i & (1ULL << e)) ? 1 : 0;
                //(1ULL << e) = 2^e
                ++e;
            } 
            //Forzamos la reflexividad en la matriz para que cumpla con ser un preorden.
            else
                M[x * T + y] = 1; 
            /*Si no se fuerza la reflexividad en la matriz se tendrian que hacer mas calculos, por ejemplo: A = {1,2,3}
            |A| = 3, si la forzamos tendriamos que evaluar 2^(3^2 - 3) = 64 matrices, mientras que si lo hacemos
            caso por caso tendriamos que evaluar 2^(2^3) = 256 matrices.*/
        }
        /*************************************************************************************************************/
        //Evaluamos la transitividad suponiendo que es verdadera, si no se cumple guardamos los indices donde fallo.
        int transitiva = 1;
        // Variables para capturar si falla la transitividad
        int fx = -1, fy = -1, fz = -1;
        for (int x = 0; x < T && transitiva; x++) 
        for (int y = 0; y < T && transitiva; y++) 
        for (int z = 0; z < T && transitiva; z++) 
            if ((M[x * T + y] == 1) && (M[y * T + z] == 1) && (M[x * T + z] == 0)) 
            {
                transitiva = 0;
                fx = x; fy = y; fz = z; // Capturamos los índices responsables del fallo
            }
        /*************************************************************************************************************/
        
        pthread_mutex_lock(&mutex_impresion);
        if(transitiva)
        {
            printf("----------------------------------------\n");
            ++topologias;
            printf("Topologia encontrada\n");
            printf("Matriz generadora:\n");
            //Mostramos la matriz obtenida.
            for (int r = 0; r < T; ++r) 
            {
                for (int c = 0; c < T; ++c)
                    printf("%d ", M[r * T + c]);
                printf("\n");
            }
            
            int C[T];
            //Realizamos todas union(disyuncion/suma) de vecindarios.
            for (int c = 0; c < T; ++c)
                C[c] = Columnas(c, M);

            int total_subconjuntos = 1 << T; //Subconjuntos generador por P(X) donde X es el conjunto ingresado(el arreglo: elementos[][])
            printf("CONJUNTO GENERADO: \n{");
            
            //Calculamos e imprimimos los subconjuntos generador por P(P(X))
            for (int A = 0; A < total_subconjuntos; A++) 
            {
                int disyuncion = 0;
                for (int c = 0; c < T; ++c)
                    if (A & (1 << c)) disyuncion |= C[c];
                //Cuando un elemento pertenece al conjunto A realizamos la disyuncion(union) 
                //o si prefieren verlo como suma de los elementos de la columna en la que se encuentra.
                //Si la disyuncion es igual a A significa que se cumple el ultimo requisito para que 
                //se pueda generar una topologia por lo imprimimos.
                if (disyuncion == A) 
                {
                    imp_elemento(A);
                    if(A < total_subconjuntos-1)
                        putchar(',');
                }
            }
            printf("}\n\n");
        } 
        else 
        {
            printf("----------------------------------------\n");
            printf("%d Matriz Invalida . No genera topologia:\n", ++no_topologias);
            for (int r = 0; r < T; ++r) 
            {
                for (int c = 0; c < T; ++c) 
                    printf("%d ", M[r * T + c]);
                printf("\n");
            }
            printf("\nMotivo: Falla la Transitividad ya que:\n");
            printf("- El elemento x%d se relaciona con x%d (M[%d][%d] = 1)\n", fx, fy, fx, fy);
            printf("- El elemento x%d se relaciona con x%d (M[%d][%d] = 1)\n", fy, fz, fy, fz);
            printf("-> PERO    x%d NO se relaciona con x%d (M[%d][%d] = 0)\n\n", fx, fz, fx, fz);
        }
        pthread_mutex_unlock(&mutex_impresion);
    }

    free(M); 
    return NULL;
}

//Funciones para validar los elementos que ingresa el usuario.
void Ingresa_Elementos()
{

    printf("Ingresa la cantidad de elementos: ");
    char c = '0';
    do
    {
        if(!scanf(" %d", &T))
        {
            printf("Dato invalido\n");
            while(c != '\n' && c != EOF)
                c = getchar();
        }
        if(T <= 0 || T > 8)
            printf("Ingresa un numero Natural menor o igual a 8: \n");
        
        if(T >= 6 && T <= 8)
        {
            printf("El proceso puede tardar demasiado tiempo en terminar, aproximadamente:,");
            switch(T)
            {
                case 6:
                    printf("entre 24 y 40 HORAS(°o°)\n");
                    break;
                case 7:
                    printf("entre 11 y 30 ANIOS (T-T)\n");
                    break;
                default:
                    printf("No se, posiblemente EONES de ANIOS (x_x)\n");
            }
            do
            {
                printf("Quieres continuar?(s/n): ");
                scanf(" %c", &c);
                if(c == 'n') T = -1;
                    else if(c == 's')
                        {
                            printf("Seguro(a)?");
                            scanf(" %c", &c);
                            if(c == 'n') T = -1;
                        }
            } while(c != 's' && T > 0);
            
        }  
    }while(T <= 0 || T > 8);

    char aux[6];
    printf("!IMPORTANTE!\n");
    printf("Solo se tomara en cuenta los primeros 5 caracteres alfanumericos\n");
    for(int i = 0; i < T; ++i)
    {
        int j = 0, r = 0;
        do
        {
            printf("Ingresa el %d elemento:", i+1);
            fflush(stdin);
            while((c = getchar()) != '\n')
                if(isalnum(c)) 
                {
                    aux[j] = c;
                    if(j < 5)
                        ++j;
                }
            aux[j] = '\0';
            if(j == 0)
            {
                printf("Ingresa almenos un elemento\n");
                continue;
            }
            strcpy(elementos[i],aux);
            r = Compara(i);
            if(!r)
                printf("YA INGRESASTE ESTE ELEMENTO!\n");
        } while(!r);
    }
}
int Compara(int j)
{
    if(j > 0)
    for(int i = 0; i < j; ++i)
    {   
        if(strcmp(elementos[i],elementos[j]) == 0)
            return(0);
    }
    return(1);
} 