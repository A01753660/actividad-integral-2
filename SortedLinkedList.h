// SortedLinkedList.h

// Lista ligada ordenada (Sorted Linked List) hecha con nodos doblemente
// ligados y dos nodos centinela (head y bottom). Cada elemento que se agrega
// se mete directo en su lugar, asi la lista siempre esta ordenada de menor a
// mayor. Tambien trae su propio iterador para poder recorrerla y una funcion
// find para buscar con un predicado.
//
// Equipo 1
// Alan Farid Hernández Sanmartín - A01753660
// Alejandro Sánchez Calderón - A01754913
// Fecha 06-11-2026
#ifndef SortedLinkedList_H_INCLUDED
#define SortedLinkedList_H_INCLUDED
#include <iostream>
#include "Node.h"

using std::cout;
//Declaramos el iterador antes porque la lista lo usa, pero su definicion
//esta mas abajo
template <typename T>
class ListIterator;

template <typename T>
class SortedLinkedList {
private:
    int list_size;
    Node<T> *head;
    Node<T> *bottom;

    // El iterador es friend para que pueda usar los nodos privados de la lista
    friend class ListIterator<T>;

public:
    // Alias para poder escribir SortedLinkedList<T>::iterator
    typedef ListIterator<T> iterator;

    // Constructor: crea la lsta vacia, que es solo head y bottom (los
    //centinelas) apuntandose entre ellos. Con los centinelas no tenemos que
    // checar casos especiales de lista vacia al insertar.
    SortedLinkedList(): list_size{0}, head{new Node<T>{}}, bottom{new Node<T>{}} {
        head->next = bottom;
        bottom->prev = head;
    }

    // Imprime todos los elememtos de la lista, uno por linea. Nos sirvio para
    //probar que la lista se estuviera ordenando bien.
    void print() {
        Node<T> *i = head->next;
        while (i != bottom) {
            cout << i->data << " ";
            cout << "\n";
            i = i->next;
        }
    }

    // Agrega un elemento en la posicion que le toca para que la lista siga
    // ordenada de menor a mayor (usa el operador < de T).
    // Empezamos desde el final (bottom) y nos vamos hacia atras con prev
    // mientras el de antes sea mayor que el nuevo. Lo hicimos asi para que si
    // hay elementos iguales, el nuvo quede despues de los que ya estaban y se
    // respete el orden en que llegaron. Como la bitacora viene por fecha, los
    // eventos de una misma ip destino tambien quedan en orden cronologico.
    // Parametros:
    //   element: el elemento que vamos a agregar.
    void add(T &element) {
        Node<T> *i = bottom;
        while (i->prev != head && element < i->prev->data) {
            i = i->prev;
        }

        // El nuevo nodo va entre i->prev e i. Su next y prev ya los pone el
        // constructor, aqui solo falta que los vecinos apunten al nuevo nodo
        Node<T> *newNode = new Node<T>{element, i, i->prev};
        i->prev->next = newNode;
        i->prev = newNode;
        ++list_size;
    }

    // Quita el primer elemento de la lista y libera su nodo. Ojo: la lista no
    // debe estar vacia, porque si no borrariamos el centinela bottom.
    // Regresa:
    //   Una copia del elemento que quitamos.
    T remove_first() {
        Node<T> *r = head->next;
        head->next = r->next;
        r->next->prev = head;
        --list_size;
        T removed{r->data};
        delete r;
        return removed;
    }

    // Busca el primer elemento que cumpla con el predicado fun. Es busqueda
    // secuencial: empieza desde el inicio y avanza hasta que lo encuentra.
    // Parametros:
    //   fun: funcion (en nuestro caso una lambda) que recibe un elemento y
    //        regresa true si es el que estamos buscando.
    // Regresa:
    //   Iterador al primer element que cumple, o end() si no hay ninguno.
    template <typename F>
    iterator find(F &fun) {
        Node<T> *i = head->next;
        while (i != bottom && !fun(i->data)) {
            i = i->next;
        }
        return iterator{i};
    }

    // Borra todos los elementos de la lista (los centinelas se quedan).
    void clear() {
        while (list_size > 0) {
            remove_first();
        }
    }

    // Regresa:
    //   Cuantos elementos tiene la lista.
    int get_size() {
        return list_size;
    }

    // Regresa:
    //   true si la lista esta vacia, false si tiene al menos un elemento.
    bool is_empty() {
        return list_size == 0;
    }

    // Regresa:
    //   Iterador al primer elemento. Si la lista esta vacia es igual a end().
    iterator begin() {
        return iterator{head->next};
    }

    // Regresa:
    //   Iterador a bottom, o sea uno despues del ultimo elemento. Sirve para
    //   saber cuando ya terminamos de recorrer la lista.
    iterator end() {
        return iterator{bottom};
    }

    // Destructor:borra todos los nodos y luego los dos centinelas para no
    // dejar memoria sin liberar.
    ~SortedLinkedList() {
        clear();
        delete head;
        delete bottom;
    }
};
// Iterador de la SortedLinkedList.Basicamente guarda un apuntdor al nodo
// actual y lo mueve con next/prev.
template <typename T>
class ListIterator {
private:
    // Nodo en el que esta parado el iterador
    Node<T> *current;
public:
    // Constructor: el iterador empieza en el nodo que le pasamos.
    // Parametros:
    //   i: nodo donde empieza el iterador.
    ListIterator(Node<T> *i): current{i} {
    }

    // Sobrecarga de ==, dos iteradores son iguales si apuntan al mismo nodo.
    // Parametros:
    //   rhs: el otro iterador.
    // Regresa:
    //   true si apuntan al mismo nodo, false si no.
    bool operator==(const ListIterator<T> &rhs) const {
        return current == rhs.current;}
    // Sobrecarga de !=, la usamos en los ciclos para ver si ya llegamos a end().
    // Parametros:
    //   rhs: el otro iterador.
    // Regresa:
    //   true si apuntan a nodos diferentes, false si no.
    bool operator!=(const ListIterator<T> &rhs) const {
        return current != rhs.current;
    }

    // Sobrecarga de * para obtener el dato del nodo actual.
    // Regresa:
    //   Referencia al dato, asi se puede leer o modificar.
    T& operator*() {
        return current->data;
    }

    // ++it (pre-incremento): avanza al siguiente nodo.
    // Regresa:
    //   El mismo iterador ya avanzado.
    ListIterator& operator++() {
        current = current->next;
        return *this;
    }

    // it++ (post-incremento): avanza al siguiente nodo pero regresa una copia
    // de como estaba antes de avanzar.
    // Regresa:
    //   Copia del iterador antes de moverse.
    ListIterator operator++(int) {
        ListIterator<T> clone(this->current);
        current = current->next; return clone;}
    //--it (pre-decremento): se regresa al nodo anterior.
    // Regresa:
    //   El mismo iterador ya movido.
    ListIterator& operator--() {
        current = current->prev;
        return *this;
    }
    // it-- (post-decremento): se regresa al nodo anterior pero regresa una
    // copia de como estaba antes.
    // Regresa:
    //   Copia del iterador antes de moverse.
    ListIterator operator--(int) {
        ListIterator<T> clone(this->current);
        current = current->prev;
        return clone;
    }
};

#endif // SortedLinkedList_H_INCLUDED
