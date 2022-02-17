#include "util.hpp"
#include <stdexcept>
#include <sstream>
#include <algorithm>

double factorial(double num)
{
    if (num < 0)
        error("no existe el factorial de un numero negativo");
    double result = 1;
    for (double i = 1; i <= num; i++)
    {
        if (result > std::numeric_limits<double>::max() / i)
            error("infinity");
        result *= i;
    }
    return result;
}

void error(const std::string& e){
    throw std::runtime_error(e);
}

void error(const std::string& s, int i)
{
	std::ostringstream os;
	os << s <<": " << i;
	error(os.str());
}
void error(const std::string& s, const std::string& s2)
{
	error(s+s2);
}

bool narrow_cast(const double a)
{
  int r = static_cast<int>(a);
  if(static_cast<double>(r) != a) return true;
  
  return false;
}

std::string &ltrim(std::string &str) // elimina espacios por la izquierda
{
    auto it2 = std::find_if(str.begin(), str.end(), [](char ch) {
        return !std::isspace<char>(ch, std::locale::classic());
    });
    str.erase(str.begin(), it2);
    return str;
}

std::string &rtrim(std::string &str) // elimina espacios por la derecha
{
    auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<unsigned char>(ch, std::locale::classic()); });
    str.erase(it1.base(), str.end());
    return str;
}

std::string trim(std::string str) // elimina espacios al inicio y al final
{
    return ltrim(rtrim(str));
}