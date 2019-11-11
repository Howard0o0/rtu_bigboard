#include "ioDev.h"
#include "string.h"

static PT_IODev g_ptIODevHead;

int RegisterIODev(PT_IODev ptIODev)
{
	if( ptIODev == NULL)
	{
		return -1;
	}
	g_ptIODevHead = ptIODev;

	return 0;
}

void ShowIODevs(void)
{
	PT_IODev ptTmp = g_ptIODevHead;
	printf("IODevs : %s\n", ptTmp->name);
}

PT_IODev getIODev(void)
{
    return g_ptIODevHead;
}

