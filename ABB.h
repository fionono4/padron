#ifndef ABB_H_INCLUDED
#define ABB_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXELECTORES 2000
//P adron´ ⊆ DNI × N ombre Apellido × Domicilio × Cod. ´ P ostal × Mesa × Circuito

typedef struct {
    int dni;
    char apellidoynombre[80];
    char domicilio[90];
    int codpostal;
    int mesa;
    int circuito;
} PadronABB;
// Variables globales para el costo de las operaciones
float cantActPtrosAlta = 0.0, MaxAlta = 0.0;
int cantAltas = 0;
float cantActPtrosBaja = 0.0, MaxBaja = 0.0;
int cantBajas = 0;
int comparacionesEvocar = 0, cantEvocaciones = 0, MaxComparaciones = 0;


typedef struct nodoA {
    PadronABB datos;
    struct nodoA *izq;
    struct nodoA *der;
} NodoA;

typedef struct {
    NodoA *raiz;
    NodoA *cur;
    NodoA *padre;
} Arbol;

void init_ABB(Arbol *a) {
    a->raiz = NULL;
    a->cur = NULL;
    a->padre = NULL;
}


void Localizar_ABB(Arbol *a, int dni, int *exito, int *comparaciones) {
    *comparaciones = 0;
    a->cur = a->raiz;
    a->padre = NULL;

    while (a->cur != NULL) {
        (*comparaciones)++;
        if (a->cur->datos.dni==dni) {
            *exito = 1;
            return;
        }
        a->padre = a->cur;
        if (a->cur->datos.dni < dni) {
            a->cur = a->cur->der;
        } else {
            a->cur = a->cur->izq;
        }
    }
    *exito = 0;
}

void Alta_ABB(Arbol *a, PadronABB nuevoElector, int *exito) {
    int exitoLocalizar = 0, comparaciones = 0;
    float actualizacionestemp = 0.0;
    Localizar_ABB(a, nuevoElector.dni, &exitoLocalizar, &comparaciones);

    NodoA *nuevoNodo = (NodoA *)malloc(sizeof(NodoA));
    if (nuevoNodo == NULL) {
        *exito = 0;  // Error en la asignación de memoria
        return;
    }
    nuevoNodo->datos = nuevoElector;
    nuevoNodo->der = NULL;
    nuevoNodo->izq = NULL;

    if (a->raiz == NULL) {
        a->raiz = nuevoNodo;  // El árbol estaba vacío
        actualizacionestemp+= 0.5;  // Actualización de puntero
        *exito = 1;
    } else if (exitoLocalizar) {
        *exito = 2;  // codigo duplicado
        free((void *)nuevoNodo);  // Liberar el nodo asignado si es duplicado
    } else {
        if (nuevoElector.dni < a->padre->datos.dni) {
            a->padre->izq = nuevoNodo;
        } else {
            a->padre->der = nuevoNodo;
        }
        actualizacionestemp+=0.5;  // Actualización de puntero
        *exito = 1;
    }

    if (*exito == 1) {
        cantAltas++;  // Incrementar el número de altas exitosas
        cantActPtrosAlta += actualizacionestemp;  // Sumar costo por actualizaciones de punteros
        if (actualizacionestemp > MaxAlta) {
            MaxAlta = actualizacionestemp;  // Actualizar el máximo de actualizaciones
        }
    }
}

int Baja_ABB(Arbol *a, PadronABB elector) {
    int exitoLocalizar = 0;
    int comparaciones = 0;
    float actualizacionestemp = 0.0;

    // Localizar el nodo a eliminar
    Localizar_ABB(a, elector.dni, &exitoLocalizar, &comparaciones);

    if (!exitoLocalizar) return 0;  // elector no encontrado

    NodoA *nodoeliminar = a->cur;


    if (nodoeliminar->datos.dni != elector.dni ||
        strcasecmp(nodoeliminar->datos.apellidoynombre, elector.apellidoynombre) !=0 ||
        strcasecmp (nodoeliminar->datos.domicilio, elector.domicilio) !=0 ||
        nodoeliminar->datos.codpostal != elector.codpostal ||
        nodoeliminar->datos.mesa != elector.mesa ||
        nodoeliminar->datos.circuito != elector.circuito) {
        return 0;
    }

    // Caso 1: Nodo hoja (sin hijos)
    if (nodoeliminar->izq == NULL && nodoeliminar->der == NULL) {
        if (a->padre == NULL) {
            a->raiz = NULL;
        } else if (a->padre->izq == nodoeliminar) {
            a->padre->izq = NULL;
        } else {
            a->padre->der = NULL;
        }
        free(nodoeliminar);
        actualizacionestemp+=0.5;
    }
    // Caso 2: Nodo con un solo hijo
    else if (nodoeliminar->izq == NULL || nodoeliminar->der == NULL) {
        NodoA *hijo = (nodoeliminar->izq != NULL) ? nodoeliminar->izq : nodoeliminar->der;

        if (a->padre == NULL) {
            a->raiz = hijo;
        } else if (a->padre->izq == nodoeliminar) {
            a->padre->izq = hijo;
        } else {
            a->padre->der = hijo;
        }
        free(nodoeliminar);
        actualizacionestemp+=0.5;
    }
    // Caso 3: Nodo con dos hijos, usar menor de los mayores
    else {
        NodoA *padreMenorMayor = nodoeliminar;
        NodoA *menorMayor = nodoeliminar->der;

        // Buscar el menor del subárbol derecho
        while (menorMayor->izq != NULL) {
            padreMenorMayor = menorMayor;
            menorMayor = menorMayor->izq;
        }

        // Copiar datos al nodo a eliminar
        nodoeliminar->datos = menorMayor->datos;

        // Eliminar el nodo del menor de los mayores
        if (padreMenorMayor == nodoeliminar) {
            padreMenorMayor->der = menorMayor->der;
        } else {
            padreMenorMayor->izq = menorMayor->der;
        }

        free(menorMayor);
        actualizacionestemp++;  // Copia + ajuste punteros
    }

    // Costos de baja
    cantBajas++;
    cantActPtrosBaja += actualizacionestemp;
    if (actualizacionestemp > MaxBaja) {
        MaxBaja = actualizacionestemp;
    }
    return 1;
}


void MostrarEstructura_ABB(Arbol *a) {
    if (a->raiz == NULL) {
        printf("El arbol esta vacio.\n");
        return;
    }

    NodoA *pila[MAXELECTORES];  // Pila para manejar hasta 2000 electores
    int tope = -1;
    int contador = 0;
    // Iniciar con la raíz del árbol
    pila[++tope] = a->raiz;
    while (tope != -1 && contador < MAXELECTORES) {
        NodoA *actual = pila[tope--];
        printf("Elector: %d:\n", contador + 1);
        printf("D.N.I: %d\n", actual->datos.dni);
        printf("Apellido y Nombre: %s\n", actual->datos.apellidoynombre);
        printf("Domicilio: %s\n", actual->datos.domicilio);
        printf("Código postal: %d\n", actual->datos.codpostal);
        printf("Mesa: %d\n", actual->datos.mesa);
        printf("Circuito: %d\n", actual->datos.circuito);

        if (actual->izq != NULL) {
            printf("DNI del Hijo izquierdo: %d\n", actual->izq->datos.dni);
        } else {
            printf("DNI del Hijo izquierdo: NULL\n");
        }
        if (actual->der != NULL) {
            printf("DNI del Hijo derecho: %d\n", actual->der->datos.dni);
        } else {
            printf("DNI del Hijo derecho: NULL\n");
        }

        printf("----------------------\n");
        contador++;

        if (contador % 5 == 0) {
            printf("Presione Enter para continuar...\n");
            getchar();
        }
        // Primero empujamos el hijo derecho, luego el izquierdo para procesarlos en orden preorden
        if (actual->der != NULL) {
            pila[++tope] = actual->der;
        }
        if (actual->izq != NULL) {
            pila[++tope] = actual->izq;
        }
    }
}

 int Evocar_ABB(Arbol *a, int dni, int *exito, PadronABB *electorEncontrado) {
    int comparacionestemp = 0;
    // Localizar el nodo por DNI
    Localizar_ABB(a, dni, exito, &comparacionestemp);
    if (*exito) {
        *electorEncontrado = a->cur->datos;
        // Actualizar costos de evocación exitosa
        comparacionesEvocar += comparacionestemp;
        if (comparacionestemp > MaxComparaciones) {
            MaxComparaciones = comparacionestemp;
        }
        cantEvocaciones++;
        return 1;  // Evocación exitosa
    } else {
        return 0;  // Evocación no exitosa
    }
}


#endif // ABB_H_INCLUDED
