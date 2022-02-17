#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <fstream>

double expression(); // declaration so that primary() can call expression()
double primary();
double term();
double funcionesMatematicas();
std::string showhelp();

// TODO: usar un namespace?
constexpr char numberKey = '8'; // t.kind == numberKey means that t is a numberKey Token
constexpr char quitKey = 'q';
constexpr char printKey = ';';
const std::string prompt = "> ";
const char outputKey = '=';
const char variable_value_key = 'a';
const char declarationKey = 'L';
const std::string str_declaration = "let";
const char underscoresKey = '_';
const char new_value_key = 'N';
const char constantKey = 'C';
const char helpKey = 'h';
const std::string str_constante = "const";
const std::string str_help = "help";
const std::string str_quit = "quit";
const std::string str_potencia = "pow";
const std::string str_raiz = "sqrt";
const char potenciaKey = 'P';
const char raizKey = 'R';
const std::string indent = std::string(3, ' ');
const char tab = '\t';
const std::string indent_test = std::string(3, ' ');
const std::string indent_salida = std::string(1, ' ');
const std::string nombre_archivo_escribir = "resultado_test";

class Token
{
public:
    bool help=false;
    char kind=' ';    // what kind of token
    double value=0; // for numberKeys: a value
    std::string name="";
    Token() = default;
    Token(char ch) // make a Token from a char
        : kind(ch)
    {
    }
    Token(char ch, double val) // make a Token from a char and a double
        : kind(ch), value(val)
    {
    }
    Token(char ch, std::string n) : kind{ch}, name{n} {}
};

class Token_stream
{
public:
    char ultimo_caracter(){
        return buffer.kind;
    }
    std::string devolver_name(){
        return buffer.name; // para let x=5t
    }
    bool get_help(){
        return buffer.help;
    }
    void establecer_punto_coma(){
        buffer.kind = printKey;
    }
    void eliminar_full(){
        full = false;
    }
    std::stringstream nuevo_stream;
    Token_stream();        // make a Token_stream that reads from cin
    Token get();           // get a Token (get() is defined elsewhere)
    void putback(const Token& t); // put a Token back
    void ignore(char c);   // discard characters up to and including a c
private:
    bool full;    // is there a Token in the buffer?
    Token buffer; // here is where we keep a Token put back using putback()
};

class archivo
{
    std::ofstream file;
    std::string nombre_test;
    std::string nombre_salida;
    bool eof = false;

public:
    archivo& asignar_test(const std::string& name){
        nombre_test = name;
        return *this;
    }
    std::string devolver_test() const{
        return nombre_test;
    }
    archivo& asignar_salida(const std::string& name){
        nombre_salida = name;
        return *this;
    }
    std::string devolver_salida() const{
        return nombre_salida;
    }
    void abrir_archivo(const std::string &name)
    {
        file.open(name, std::ofstream::app);
        if (!file)
            throw std::runtime_error("no se pudo abrir el archivo");
    }
    void escribir_archivo_test(const std::string &nombre_archivo)
    {
        const std::string linea = "============================================================================\n";
        abrir_archivo(nombre_archivo);
        file << linea << "test: " << indent_test << devolver_test();
        cerrar_archivo();
    }
    void escribir_texto_salida(){
        abrir_archivo(nombre_archivo_escribir);
        file << "salida: " << indent_salida;
        cerrar_archivo();
    }
    void escribir_archivo_salida(const std::string& nombre_archivo){
        abrir_archivo(nombre_archivo);
        file << indent_salida << devolver_salida();
        cerrar_archivo();
    }
    void cerrar_archivo(){
        file.close();
    }
    void activar_eof(){
        eof = true;
    }
    void desactivar_eof(){
        eof = false;
    }
    bool estado_eof(){
        return eof;
    }
    ~archivo()
    {
        if (file.is_open()) // por seguiridad ya que ofstream utiliza el famoso RAII
            cerrar_archivo();
    }
};

class Variable
{

public:
    std::string name;
    double value;
    bool constant;
};

class Symbol_table
{
    std::vector<Variable> var_table;

public:
    bool is_declared(const std::string&);
    double get(const std::string&);
    void set(const std::string&, double);
    double declare(const std::string&, double, bool);
};