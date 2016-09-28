#ifndef MYLIB_H
#define MYLIB_H

#include <string>

namespace mylib
{

class MyClass
{
public:
    MyClass() = default;
    virtual ~MyClass() = default;
    
    std::string get_message();

private:

};

} // namespace mylib

#endif /* MYLIB_H */
