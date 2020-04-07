#include <iostream>
#include "Wrapper/Wrapper.hpp"

int main()
{
    WTS::Wrapper::Querier querier;

    std::vector<WTS::Wrapper::Session> sessions(querier.QuerySessions());

    return 0;
}
