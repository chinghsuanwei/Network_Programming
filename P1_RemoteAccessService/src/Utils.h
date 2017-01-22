#ifndef UTILS_H_
#define UTILS_H_


//#define DEBUG
#define SOCKET_COMMUTICATE
#define ENVPATHSIZE 256
#define BUFSIZE 1024
#define INPUTSIZE 16384

struct FwdPipe
{
    int rpipe;
    int wpipe;
};

enum ForwardType{NONE, FORWARD_STDOUT, FORWARD_STDERR};
#endif
