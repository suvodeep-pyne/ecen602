#include <iostream>

#include "connection.h"
#include "server.h"

int main ()
{
    Connector conn(true);
    conn.setup("localhost", "5000");
    conn.startListener();

    return 0;
}
