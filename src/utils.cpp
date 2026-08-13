#include "ft_irc.hpp"
#include <climits>
#include <cctype>

bool isPositiveInteger(const std::string &value)
{
    if (value.empty())
        return false;
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
    {
        if (!std::isdigit(static_cast<unsigned char>(*it)))
            return false;
    }
    if (value.size() > 1 && value[0] == '0')
        return false;
    long long n = 0;
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
    {
        n = n * 10 + (*it - '0');
        if (n > INT_MAX)
            return false;
    }
    return true;
}
