#include <iostream>

#include "client.h"

int main ()
{
    Connector conn(true);
    conn.setup("localhost", "5000");
    return 0;
}
