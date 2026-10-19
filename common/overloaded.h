#ifndef OVERLOADED_H
#define OVERLOADED_H

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

#endif
