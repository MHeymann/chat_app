#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "packet.h"

char *serialize(packet_t *packet, int *psize);

packet_t *deserialize(char *bytes); 

#endif
