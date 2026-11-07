// Actividad Integral 2 - Estructuras de Datos Lineales
//
// Programa que lee la bitacora que nos quedo de la Actividad Integral 1 (ya
// ordenada por fecha) y va metiendo cada evento a una lista ligada ordenada
// por ip destino, como si los eventos fueran llegando en tiempo real. Al final
// guarda la lista en otro archivo y le pide al usuario una ip para mostrar
// todos los eventos que tienen esa ip destino.
//
// Equipo 1
// Alan Farid Hernández Sanmartín - A01753660
// Alejandro Sánchez Calderón - A01754913
// Fecha 06-11-2026

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
#include "SortedLinkedList.h"

using namespace std;

// Archivo de entrada, es la bitacora que salio de la Actividad Integral 1
const string INPUT_FILE = "bitacora_ordenada.csv";

// Aqui se guarda la bitacora ya ordenada por ip destino
const string OUTPUT_FILE = "bitacora_ordenada_ip.csv";

// Cada linea de la bitacora tiene 8 campos separados por comas
const int FIELD_COUNT = 8;

// Un octeto de ip va de 0 a 255, lo usamos para validar la ip del usuario
const int MAX_OCTET = 255;

// Struct para una direccion ip, guardamos cada octeto como entero para poder
// compararlos como numeros
struct ip {
    int o1;
    int o2;
    int o3;
    int o4;
};

// Struct para un evento de la bitacora (una conexion entre dos nodos)
// ts: fecha y hora del evento como struct tm de ctime (docs: https://cplusplus.com/reference/ctime/tm/)
// ip_o: ip del nodo origen
// port_o: puerto del nodo origen
// domain_o: dominio del nodo origen
// ip_d: ip del nodo destino (con esta ordenamos la lista)
// port_d: puerto del nodo destino
// domain_d: dominio del nodo destino
struct event {
    struct tm ts;
    struct ip ip_o;
    string port_o;
    string domain_o;
    struct ip ip_d;
    string port_d;
    string domain_d;
};

// Sobrecarga de << para imprimir una ip como "a.b.c.d".
// En la bitacora hay campos con '-' cuando no hay ip, y como al leerlos los
// guardamos con todo en 0, aqui los volvemos a imprimir como '-'.
// Parametros:
//   os: flujo donde se va a imprimir.
//   address: la ip que queremos imprimir.
// Regresa:
//   El mismo flujo, para poder encadenar varios <<.
ostream& operator<<(ostream &os, const ip &address) {
    if (address.o1 == 0) {
        os << "-";
    } else {
        os << address.o1 << "." << address.o2 << "." << address.o3 << "."
           << address.o4;
    }
    return os;
}

// Sobrecarga de << para imprimir un evento con el mismo formato de la
// bitacora (todo separado por comas), asi el archivo de salida queda igual
// que el de entrada y se puede volver a leer.
// Parametros:
//   os: flujo donde se va a imprimir.
//   e: el evento que queremos imprimir.
// Regresa:
//   El mismo flujo, para poder encadenar varios <<.
ostream& operator<<(ostream &os, const event &e) {
    // Usamos %H:%M:%S y no %T porque con MinGW en Windows %T no jala y
    // strftime deja la cadena vacia (nos salian los eventos sin fecha ni hora)
    char dateOutput[20] = "";
    strftime(dateOutput, 20, "%d-%m-%Y,%H:%M:%S", &e.ts);
    os << dateOutput << "," << e.ip_o << "," << e.port_o << ","
       << e.domain_o << "," << e.ip_d << "," << e.port_d << ","
       << e.domain_d;
    return os;}
// Compara dos ips para saber si ip1 es menor que ip2. Vamos octeto por octeto
// de izquierda a derecha: si son diferentes ese decide, y si son iguales
// pasamos al siguiente. No las comparamos como string porque asi "10.x.x.x"
// quedaria antes que "9.x.x.x".
// Parametros:
//   ip1: primera ip.
//   ip2: segunda ip.
// Regresa:
//   true si ip1 es menor que ip2, false si no.
bool operator<(const ip &ip1, const ip &ip2) {
    if (ip1.o1 != ip2.o1) {
        return ip1.o1 < ip2.o1;
    }
    if (ip1.o2 != ip2.o2) {
        return ip1.o2 < ip2.o2;
    }
    if (ip1.o3 != ip2.o3) {
        return ip1.o3 < ip2.o3;}
    return ip1.o4 < ip2.o4;}
// Checa si dos ips son iguales. La necesitamos para la lambda de la busqueda.
// Parametros:
//   ip1: primera ip.
//   ip2: segunda ip.
// Regresa:
//   true si los 4 ocetos son iguales, false si no.
bool operator==(const ip &ip1, const ip &ip2) {
    return ip1.o1 == ip2.o1 && ip1.o2 == ip2.o2 && ip1.o3 == ip2.o3
           && ip1.o4 == ip2.o4;
}

// Sobrecarga de < para eventos (la pide la actividad). Solo comparamos la ip
// destino porque es lo que usamos para ordenar. Este es el operador que usa
// add() de la SLL para saber en que posicion va cada evento.
// Parametros:
//   e1: primer evento.
//   e2: segundo evento.
// Regresa:
//   true si la ip destino de e1 es menor que la de e2, false si no.
bool operator<(const event &e1, const event &e2) {
    return e1.ip_d < e2.ip_d;}
// Convierte un string tipo "a.b.c.d" a nuestro struct ip.
// Si viene "-" (no hay ip) o vacio, regresamos todo en 0, y despues el << lo
// imprime otra vez como '-'.
// Parametros:
//   text: la ip como string.
// Regresa:
//   El struct ip con sus 4 octetos.
ip parseIp(const string &text) {
    ip result = {0, 0, 0, 0};
    if (text == "-" || text.empty()) {
        return result;
    }
    istringstream stream(text);
    char dot = ' ';
    stream >> result.o1 >> dot >> result.o2 >> dot >> result.o3 >> dot
           >> result.o4;
    return result;
}

// Pasa la fecha y la hora de la bitacora (vienen en campos separados) a un
// struct tm. Ojo: tm guarda el anio contando desde 1900 y los meses de 0 a 11,
// por eso les restamos 1900 y 1.
// Parametros:
//   date: fecha en formato "dd-mm-aaaa".
//   time: hora en formato "hh:mm:ss".
// Regresa:
//   El struct tm con la fecha y hora.
tm parseTimestamp(const string &date, const string &time) {
    tm timestamp = {};
    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    char separator = ' ';

    istringstream dateStream(date);
    dateStream >> day >> separator >> month >> separator >> year;
    istringstream timeStream(time);
    timeStream >> hour >> separator >> minute >> separator >> second;
    timestamp.tm_year = year - 1900;
    timestamp.tm_mon = month - 1;
    timestamp.tm_mday = day;
    timestamp.tm_hour = hour;
    timestamp.tm_min = minute;
    timestamp.tm_sec = second;
    timestamp.tm_isdst = -1;

    return timestamp;}

// Separa una linea de la bitacora por comas y llena el evento con sus 8
// campos: fecha, hora, ip origen, puerto origen, dominio origen, ip destino,
// puerto destino y dominio destino.
// Parametros:
//   line: la linea que leimos del archivo.
//   e: el evento que vamos a llenar (va por referencia).
// Regresa:
//   true si la linea traia los 8 campos, false si venia mal.
bool parseEvent(const string &line, event &e) {
    istringstream stream(line);
    string fields[FIELD_COUNT] = {};

    for (int i = 0; i < FIELD_COUNT; i++) {
        if (!getline(stream, fields[i], ',')) {
            return false;
        }}

    // Si el archivo se guardo en Windows cada linea termina con '\r' y se
    // quedaba pegado al dominio destino, por eso lo quitamos aqui
    if (!fields[7].empty() && fields[7].back() == '\r') {
        fields[7].pop_back();
    }

    e.ts = parseTimestamp(fields[0], fields[1]);
    e.ip_o = parseIp(fields[2]);
    e.port_o = fields[3];
    e.domain_o = fields[4];
    e.ip_d = parseIp(fields[5]);
    e.port_d = fields[6];
    e.domain_d = fields[7];

    return true;
}

// Lee la bitacora linea por linea y cada evento lo agrega a la lista en ese
// momento (no leemos todo y luego ordenamos), para simular que los eventos
// van llegando en tiempo real. Como add() ya lo mete en su lugar, la lista
// esta ordenada despues de cada insercion.
// La lista la pasamos por referencia porque la SLL no tiene constructor de
// copia, y si se copira las dos copias apuntarian a los mismos nodos y se
// borrarian dos veces al destruirse.
// Parametros:
//   filename: nombre del archivo de la bitacora.
//   events: la lista donde vamos guardando los eventos.
// Regresa:
//   true si se pudo abrir el archivo, false si no.
bool readLog(const string &filename, SortedLinkedList<event> &events) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: no se pudo abrir el archivo '" << filename << "'\n";
        return false;
    }

    string line = "";
    while (getline(file, line)) {
        if (!line.empty()) {
            event e = {};
            if (parseEvent(line, e)) {
                events.add(e);
            } else {
                cerr << "Advertencia: linea con formato invalido ignorada: "
                     << line << "\n";
            }
        }
    }

    file.close();
    return true;}

// Guarda todos los eventos de la lista en el archivo de salida. Como la lista
// ya esta ordenada solo la recorremos con el iterador de principio a fin.
// Parametros:
//   filename: nombre del archivo que vamos a crear.
//   events: la lista con los eventos.
// Regresa:
//   true si se pudo crear y escribir el archivo, false si no.
bool writeLog(const string &filename, SortedLinkedList<event> &events) {
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: no se pudo crear el archivo '" << filename << "'\n";
        return false;
    }

    for (SortedLinkedList<event>::iterator it = events.begin();
         it != events.end(); ++it) {
        file << *it << "\n";
    }

    file.close();
    return true;
}

// Le pide al usuario la ip a buscar y revisa que este bien escrita: que sean
// 4 numeros separados por pntos, que cada uno este entre 0 y 255 y que no
// traiga nada extra al final.
// Parametros:
//   address: aqui se guarda la ip que escribio el usuario.
// Regresa:
//   true si la ip es valida, false si no.
bool readIp(ip &address) {
    string text = "";
    cout << "\nIngresa la ip destino a buscar (ej. 180.49.240.119): ";
    cin >> text;

    istringstream stream(text);
    char dot1 = ' ';
    char dot2 = ' ';
    char dot3 = ' ';
    char extra = ' ';
    stream >> address.o1 >> dot1 >> address.o2 >> dot2 >> address.o3 >> dot3
           >> address.o4;

    bool validFormat = !stream.fail() && dot1 == '.' && dot2 == '.'
                       && dot3 == '.' && !(stream >> extra);
    bool validRange = address.o1 >= 0 && address.o1 <= MAX_OCTET
                      && address.o2 >= 0 && address.o2 <= MAX_OCTET
                      && address.o3 >= 0 && address.o3 <= MAX_OCTET
                      && address.o4 >= 0 && address.o4 <= MAX_OCTET;

    return validFormat && validRange;
}

// Main: carga la bitacora en la SLL, la guarda ordenada en el archivo y luego
// busca y muestra los eventos de la ip que escriba el usuario.
// Regresa:
//   0 si todo salio bien, 1 si hubo error al leer el archivo o la ip.
int main() {
    SortedLinkedList<event> events{};

    cout << "Leyendo bitacora '" << INPUT_FILE
         << "' (puede tardar un poco)...\n";
    if (!readLog(INPUT_FILE, events) || events.is_empty()) {
        cout << "No se regstraron eventos. Verifica el nombre del archivo.\n";
        return 1;
    }
    cout << "Eventos insertados en la lista: " << events.get_size() << "\n";
    if (writeLog(OUTPUT_FILE, events)) { cout << "Bitacora ordenada por ip destino guardada en '" << OUTPUT_FILE<< "'\n";}

    ip target = {0, 0, 0, 0};
    if (!readIp(target)) {
        cout << "Direccion ip invalida.\n";
        return 1;
    }

    // Lambda que usamos como predicado para find: regresaa true si la ip
    // destino del evento es la que escribio el usuario (target)
    auto hasTargetIp = [&target](const event &e) {
        return e.ip_d == target;};
    SortedLinkedList<event>::iterator it = events.find(hasTargetIp);
    if (it == events.end()) {
        cout << "\nNo hay eventos registrados con la ip destino " << target
             << ".\n";
        return 0;
    }

    // Como la lista esta ordenada por ip destino, todos los eventos de esa ip
    // estan juntos. Entonces desde el primero que encontro find solo avanzamos
    // hasta que cambie la ip, sin tener que recorrer toda la lista
    cout << "\nEventos con ip destino " << target << ":\n";
    int matches = 0;
    while (it != events.end() && hasTargetIp(*it)) {
        cout << *it << "\n";
        ++matches;
        ++it;}
    cout << "\nTotal de eventos con ip destino " << target << ": " << matches
         << "\n";
    return 0;}
