
#define BACKWARD_HAS_BFD 1
#include <Core/ThirdParty/backward.hpp>

#include <stdio.h>

int main() {
    using namespace backward;
    StackTrace st;
    st.load_here(64);
    st.skip_n_firsts(2);
    Printer p;
    p.print(st, std::cout);
    return 0;
}