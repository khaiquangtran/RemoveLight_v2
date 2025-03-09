#include "./Network.h"

Network::Network()
{
}

int *Network::parseCommandStringToArray(String str, int &size)
{
    char cstr[100];
    str.toCharArray(cstr, 50);

    char *pch;
    int *pnum = new int[10];
    int index = 0;
    pch = strtok(cstr, " ");
    while (pch != NULL)
    {
      pnum[index++] = atoi(pch);
      pch = strtok(NULL, " ");
    }
    size = index;
    return pnum;
}