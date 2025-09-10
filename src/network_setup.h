#ifndef NETWORK_SETUP_H
#define NETWORK_SETUP_H

extern const IPAddress apIP;

// Declare the setup functions that main.cpp will call
void setupNetworkServices();
void startDnsTask();

#endif // NETWORK_SETUP_H
