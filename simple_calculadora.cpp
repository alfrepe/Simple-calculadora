
/* TODO: 
* borrar variables
* borrar todas las variables
* ver las variables declaradas
* usar readline de la gnu para poder usar las flechas de arriba y abajo
*/
#include "util.hpp"
#include "header/simple_calculadora.hpp"
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>

#define PRETTY_PRINT std::fixed << std::setprecision(2) 

Symbol_table symbol;
Token_stream ts; // provides get() and putback()
archivo gestion_archivo;

// The constructor just sets full to indicate that the buffer is empty:
Token_stream::Token_stream()
    : full(false), buffer(0) {} // no Token in buffer

// The putback() member function puts its argument back into the Token_stream's buffer:
void Token_stream::putback(const Token &t)
{
    if (full)
        error("putback() into a full buffer");
    buffer = t;  // copy t to buffer, llama al operador de asignacion
    full = true; // buffer is now full
}

void Token_stream::ignore(char c)
{
    if (std::cin.eof())
    {
        error("good bye, have a nice day!");
    }
    if (c == '\n' || (full && c == buffer.kind)) // para el let x
    {
        full = false;
        buffer.kind = ' ';
        return;
    }
    full = false;
    char ch = ' ';
    while (nuevo_stream && ch != '\n')
    {
        nuevo_stream.get(ch);
    }
}

Token Token_stream::get()
{
    if (full)
    { // do we already have a Token ready?
        // remove token from buffer
        full = false;
        return buffer;
    }
    char ch = ' ';
    while (nuevo_stream && ch != '\n' && std::isspace(ch))
    {
        nuevo_stream.get(ch);
    }

    switch (ch)
    {
        case ';': // for "printKey"
        case '(':
        case ')':
        case '+':
        case '=':
        case '-':
        case '*':
        case '/':
        case '}':
        case '%':
        case '{':
        case '!':
        case ',':           // para pow numeros de coma flotante
        case '\n':
            return Token(ch);

        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            nuevo_stream.putback(ch); // put digit back into the input stream
            double val = ch;
            nuevo_stream >> val;       // read a floating-point numberKey
            return Token(numberKey, val); // let '8' represent "a numberKey"
        }
        default:
        {
            //is_first_char_valid_alpha(p); // para no desrreferenciar un puntero NULL
            if (std::isalpha(static_cast<unsigned char>(ch)))
            {
                std::string s;
                s += ch;
                while (nuevo_stream.get(ch) && (std::isalpha(ch) || std::isdigit(ch) || ch == underscoresKey))
                    s += ch; // va letra por letra sumandoselo a s
                nuevo_stream.putback(ch);
                if (s == str_declaration)
                    return Token{declarationKey};
                if (s == str_constante)
                {
                    return Token{constantKey};
                }
                if (s == str_help)
                {
                    return Token{helpKey};
                }
                if (s == str_quit)
                {
                    return Token{quitKey};
                }
                if (s == str_raiz)
                {
                    return Token{raizKey};
                }
                if (s == str_potencia)
                {
                    return Token{potenciaKey};
                }
                if(ts.ultimo_caracter() == numberKey) { // 9;7xf
                    error("Bad Token");
                }
                if (symbol.is_declared(s) && ch == '=') // no aparece en el libro pero es necesario para hacer funcionar el let de definir tus propias variables
                    return Token(new_value_key, s);
                return Token{variable_value_key, s};
            }
            error("Bad token");
        }
    }
}

bool Symbol_table::is_declared(const std::string& var)
{
    // is var already in var_table
    for (const Variable &v : var_table)
    {
        if (v.name == var)
            return true;
    }
    return false;
}

double Symbol_table::declare(const std::string& var, double val, bool es_constante)
{
    var_table.push_back(Variable{var, val, es_constante});
    return val;
}

double Symbol_table::get(const std::string& s)
{
    // return the value of the Variable named s
    for (const Variable &v : var_table)
    {
        if (v.name == s)
            return v.value;
    }
    error("get: undefined variable ", s);
}

void Symbol_table::set(const std::string& s, double d)
{
    for (Variable &v : var_table)
    {
        if (v.name == s)
        {
            if (v.constant) {
                Token t = ts.get();
                ts.putback(t);
                t = ts.get();
                error(s, " is constant"); // habian problemas con const i=56 y i=5;
            }
            v.value = d;
            return;
        }
    }
    error("set: undefined variable ", s);
}

double declaration(bool es_contante = false)
{
    // assume we have seen "let"

    Token t = ts.get();
    if (symbol.is_declared(t.name))
        error(t.name, " declared twice");
    if (t.kind != variable_value_key) { // const; const\n
        ts.putback(t);
        t = ts.get();
        error("name expected in declaration");
    }
    std::string var_name = t.name;
    Token t2 = ts.get();
    if (t2.kind != '='){
        if(t2.kind == '\n') { // let x\n
            ts.putback(t2);
            t = ts.get();
            error("= missing in declaration of ", var_name);
        }
    }
    double d = expression(); // uy uy uyyyyy
    //t.name = f ya que let juan = 7f
    if(!ts.devolver_name().empty()){ // let juan=7f
        t = ts.get();
        error("valor no valido para la variable ",var_name);
    }
    symbol.declare(var_name, d, es_contante);
    return d;
}

double statement()
{
    Token t = ts.get();
    switch (t.kind)
    {
    case constantKey:
    {
        return declaration(true);
    }
    case declarationKey:
        return declaration();
    case new_value_key:
    {
        Token t2 = ts.get();
        double d = expression();
        symbol.set(t.name, d);
        return d;
    }
    default:
        ts.putback(t);
        return expression();
    }
}

void clean_up_mess(char c)
{
    ts.ignore(c);
}

// deal with + and -
double expression()
{
    double left = term(); // read and evaluate a Term
    Token t = ts.get();   // get the next token from token stream

    while (true)
    {
        switch (t.kind)
        {
        case '+':
            left += term(); // evaluate Term and add 1+
            t = ts.get();
            break;
        case '-':
            left -= term(); // evaluate Term and subtract
            t = ts.get();
            break;
        default:
            ts.putback(t); // put t back into the token stream
            return left;   // finally: no more + or -: return the answer
        }
    }
}

// deal with *, /, and %
double term()
{
    double left = funcionesMatematicas();
    Token t = ts.get(); // get the next token from token stream

    while (true)
    {
        switch (t.kind)
        {
        case '*':
            left *= funcionesMatematicas();
            t = ts.get();
            break;
        case '/':
        {
            double d = funcionesMatematicas();
            if (d == 0){
                t = ts.get();
                error("divide by zero");
                
            }
            left /= d;
            t = ts.get();
            break;
        }

        case '%':
        {
            double d = primary();
            if (d == 0){
                ts.putback(t);
                t = ts.get();
                error("divide by zero");
            }
            left = std::fmod(left, d);
            t = ts.get();
            break;
        }
        default:
            ts.putback(t); // put t back into the token stream
            return left;
        }
    }
}

double funcionesMatematicas()
{
    double left = primary();
    Token t = ts.get();
    // no necesitamos un while porque es un operador unario
    switch (t.kind)
    {
    case raizKey:
    {
        if (left < 0)
            error("raiz cuadrada negativa");
        ts.putback(t);
        t = ts.get();
        return ::sqrt(left);
    }
    case '!':
    {
        return factorial(left);
    }
    default:
        ts.putback(t);
        return left;
    }
}

// deal with numberKeys and parentheses
double primary()
{
    Token t = ts.get();
    switch (t.kind)
    {
    case '(':
    {
        double d = expression();
        t = ts.get();
        if (t.kind != ')')
            error("')' expected");
        return d;
    }

    case '{': // ahora maneja bien estas expresiones {(1+1)*3); dara un error se espera un '}' no un ')'
    {
        double d = expression();
        t = ts.get();
        if (t.kind != '}')
            error("'}' expected");
        return d;
    }
    case raizKey:
    {
        double d = primary();
        ts.putback(t);
        return d;
    }
    case potenciaKey:
    {
        t = ts.get();
        if (t.kind != '('){ // se caia el programa con pow o pow;
            ts.putback(t);
            t = ts.get();
            error("'(' expected"); // mejorar el mensaje de error
        }
        double num = expression();
        t = ts.get();
        if (t.kind != ',')
            error("error en pow, se esperaba una ,");
        double base = expression();
        if(narrow_cast(base)){
            t = ts.get();
            error("info loss");
        }
        t = ts.get();
        if (t.kind != ')')
            error("')' expected");
        return std::pow(num, base);
    }
    case variable_value_key: // no aparece en el libro pero necesario para que funcione el let para definir tus propias variables
    case new_value_key:
        return symbol.get(t.name);

    case numberKey:        // we use '8' to represent a numberKey
        return t.value; // return the numberKey's value
    case '+':
        return primary();
    case '-':
        return -primary();
    default:
        if(t.kind == '\n'){ // para manejar 7& y ( sino tengo un bonito \n
            ts.putback(t);
            error("primary expected");
        }
        else if(t.kind == helpKey){
            //showhelp();
            t.help = true;
            ts.putback(t);
        }
        else
        {
            error("primary expected"); // para el ; o varios ;;;;
        }
        //error("primary expected");       
    }
}

std::string showhelp() // estilo man de git
{
    std::ostringstream os;
    os << "OPTIONS:" << '\n';
    os << tab << "<expression>\n";
    os << tab << indent << "The usual operators +, -, *, / and % (remainder)\n";
    os << tab << indent << "Expressions are fixed-point decimal numberKeys, and\n";
    os << tab << indent << "Parentheses () and corchetes {} may be used for grouping.\n\n";
    os << tab << "let <variable name>=<expression>;\n";
    os << tab << indent << "Define a variablenames may include letter and underscoresKey\n\n";
    os << tab << "const <variable name> = <expression>;\n";
    os << tab << indent << "Define a constant\n\n";
    os << tab << "<name>=<expression>\n";
    os << tab << indent << "Change the value a variable.\n\n";
    os << tab << "q;\n";
    os << tab << indent << "Stop execution\n\n";
    os << tab << "h;\n";
    os << tab << indent << "Show help\n\n";
    os << tab << "sqrt(<expression>);\n";
    os << tab << indent << "Calculate the square root.\n\n";
    os << tab << "pow(<expression>,<expression>);\n";
    os << tab << indent << "Calculate powers.\n\n";
    std::string grande = os.str();
    std::cout << grande;
    return grande;
}

bool check_invalid_help(const std::string& cadena, const std::string& ocurrence) // cosas como help,help o 1,help o 4.help help;help,
{
    //const string mi_cadena{"let x=help\n"}; // cubre este caso: mi_cadena.at(busca+ocurrencia.length()) != '\n'
    std::string copy_cadena = cadena;
    std::string str_result = ltrim(copy_cadena);
    size_t temp = 0;
    size_t busca = str_result.find(ocurrence);
    while (busca != std::string::npos)
    {
        if (busca > 0 && str_result.at(busca - 1) != ';')
        {
            return true;
        }
        temp = busca + ocurrence.length();
        busca = str_result.find(ocurrence, temp);
        temp = (busca == std::string::npos) ? temp : busca;
    }

    if(temp > 0)
        return (str_result.at(temp) == '\n' || str_result.at(temp) == ';') ? false : true; // help;help,
    return false; // sino hay ningun help

}
//1;help;;help           help;let x=4;45,;help        1;help;help;hellp
std::string get_input()
{
    Token t;
    bool bad_statement = false;
    std::string cadena{" "};
    char caracter{' '};
    char coincidencia{' '};

    while(caracter != '\n' && !std::cin.fail()){
        coincidencia = caracter;
        caracter = std::cin.get();
        if(caracter == ';' && coincidencia == ';'){
            bad_statement = true;
        }
        cadena += caracter;
    }
    // no me gusta repetir codigo...
    if(bad_statement){
        gestion_archivo.asignar_test(cadena).escribir_archivo_test(nombre_archivo_escribir);
        t.kind = '\n';
        ts.putback(t);
        error("No puede haber mas de un ; seguido");
    }
    if(std::cin.eof()){
        gestion_archivo.activar_eof();
        gestion_archivo.asignar_test("\n").escribir_archivo_test(nombre_archivo_escribir);
        error(""); // tenemos eof o dos control_d
    }
    if(check_invalid_help(cadena,str_help)){
        gestion_archivo.asignar_test(cadena).escribir_archivo_test(nombre_archivo_escribir);
        t.kind = '\n';
        ts.putback(t);
        error("delimitador no valido para help");
    }
    gestion_archivo.asignar_test(cadena).escribir_archivo_test(nombre_archivo_escribir);
    return cadena;
}

double firstStatement(Token& t){

    std::string n = get_input();
    ts.nuevo_stream << n;
    t = ts.get();
    ts.putback(t);
    return statement();
}

bool one_help_statement(Token &t) // solament hay un help, es decir, help\n
{
    if (t.kind == helpKey)
    {
        t = ts.get();
        if (t.kind == '\n')
        { 
            // help\n
            gestion_archivo.asignar_salida(showhelp()).escribir_archivo_salida(nombre_archivo_escribir);
            return true;
        }
        t.help = true; // help;
        ts.putback(t);
        t = ts.get();
    }
    return false;
}

void finalStatement(Token& t, double result){
    if (t.kind == '\n')
    { 
        // sin el else if me entra al else y por tanto al cout << result con help
        clean_up_mess(t.kind);
    }
    else if (ts.ultimo_caracter() == helpKey)
    {
        //showhelp(); // para 7;help
        gestion_archivo.asignar_salida(showhelp()).escribir_archivo_salida(nombre_archivo_escribir);
        clean_up_mess(t.kind);
    }
    else if (t.kind != helpKey)
    {
        std::cout << '=' << PRETTY_PRINT << result << '\n';
        std::string r = '=' + std::to_string(result) + '\n';
        gestion_archivo.asignar_salida(r).escribir_archivo_salida(nombre_archivo_escribir);
    }
}

void multipleStatements(Token &t, double &result)
{
    while (ts.ultimo_caracter() == printKey)
    { 
        // maneja 7;3;4;1
        if (ts.get_help())
        {
            // help; o help;4
            gestion_archivo.asignar_salida(showhelp()).escribir_archivo_salida(nombre_archivo_escribir);
            if (t.kind == '\n')
                break; // para 8;help;help
        }
        else
        {
            std::cout << '=' << PRETTY_PRINT << result << '\n'; // para expresiones como 7&7; 7;
            std::string r = '=' + std::to_string(result) + '\n';
            gestion_archivo.asignar_salida(r).escribir_archivo_salida(nombre_archivo_escribir); // convertir entero a string
        }
        t = ts.get(); // expresiones como 7*7;a

        if (t.kind == '\n')
        {
            break;
        }
        ts.putback(t);
        result = statement();   //6;9
        Token nuevo = ts.get();
        if (nuevo.help)
        {
            gestion_archivo.asignar_salida(showhelp()).escribir_archivo_salida(nombre_archivo_escribir);
            nuevo = ts.get();
            ts.putback(nuevo);
            if (nuevo.kind != '\n')
            {
                nuevo = ts.get(); // en busca de \n
                t = ts.get();
                if (t.kind == helpKey)
                {
                    nuevo.help = true;
                    ts.putback(nuevo);
                    nuevo = ts.get();
                    t = ts.get();
                    continue;
                }
                if (t.kind == '\n')
                { 
                    // para 8;help;
                    break;
                }
                ts.putback(t);
                result = statement(); // para help;help;f o 23;help;f
                nuevo = ts.get();
            }
        }
        if (nuevo.kind == '\n')
        { 
            // maneja 7;12 o 7;help
            clean_up_mess(nuevo.kind);
            break; // 7;56
        }
    }
}

// 7;help;4;
void calculate() // problema con la h de help mirar si se puede poner en primary problema con help;
{
    while (std::cin)
    {
        try
        {
            std::cout << prompt;
            Token t;
            double result = firstStatement(t);
            gestion_archivo.escribir_texto_salida();
            t = ts.get();
            if(one_help_statement(t)) {
                continue;
            }
            if(t.kind != printKey && t.kind != '\n'){
                while(t.kind != printKey && t.kind != helpKey){ // expresiones complicadas como 3+15*9(--28+1/93);
                    t = ts.get();
                    ts.putback(t);
                    result = statement();
                }
            }
            else if(t.kind == printKey){ // varias expresiones como 7&7; 7&1; 7&7; f / 1;2
                multipleStatements(t,result);
            }
            else if(t.kind == '\n'){ // expresiones tales como 5 o 6&6
                std::cout << '=' << PRETTY_PRINT << result << '\n';
                std::string r = '='+std::to_string(result)+'\n';
                gestion_archivo.asignar_salida(r).escribir_archivo_salida(nombre_archivo_escribir);
                clean_up_mess(t.kind);
            }
            finalStatement(t,result);
        }
        catch (std::exception &e)
        {
            gestion_archivo.escribir_texto_salida();
            std::cerr  << e.what()  << '\n';
            std::string r {e.what()};
            if(!gestion_archivo.estado_eof())
                r += '\n';
            gestion_archivo.desactivar_eof();
            gestion_archivo.asignar_salida(r).escribir_archivo_salida(nombre_archivo_escribir);
            clean_up_mess(ts.ultimo_caracter());
        }
    }
}

int main()
{
    try
    {
        /* define_name("pi", 3.1415926535,true);
        define_name("e", 2.7182818284,true); */
        calculate();
        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
        std::string r {e.what()};
        r += '\n';
        gestion_archivo.asignar_salida(r).escribir_archivo_salida(nombre_archivo_escribir);
        return 1;
    }
    catch (...)
    {
        std::cerr << "Oops: unknown exception!\n";
        return 2;
    }
    return 0;
}