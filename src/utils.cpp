#include "ft_irc.hpp"
#include <climits>
#include <cctype>

std::string toLowerString(const std::string &value)
{
    std::string lowered = value;
    for (std::string::iterator it = lowered.begin(); it != lowered.end(); ++it)
    {
        unsigned char c = static_cast<unsigned char>(*it);
        if (c == '[' || c == '{')
            *it = '[';
        else if (c == ']' || c == '}')
            *it = ']';
        else if (c == '\\' || c == '|')
            *it = '\\';
        else
            *it = static_cast<char>(std::tolower(c));
    }
    return lowered;
}

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

bool isValidNickname(const std::string &nick)
{
    if (nick.empty())
        return false;
    if (nick.find(' ') != std::string::npos)
        return false;
    if (nick.size() > 9)
        return false;
    char first = nick[0];
    bool isFirstValid = isalpha(first) || first == '[' || first == ']' || first == '\\' ||
                        first == '`' || first == '^' || first == '{' || first == '}' ||
                        first == '|' || first == '_';
    if (!isFirstValid)
        return false;
    for (size_t i = 1; i < nick.size(); i++)
    {
        char c = nick[i];
        bool valid = isalnum(c) || c == '[' || c == ']' || c == '\\' ||
                    c == '`' || c == '^' || c == '{' || c == '}' ||
                    c == '|' || c == '_' || c == '-';
        if (!valid)
            return false;
    }

    return true;
}
