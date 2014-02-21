#include "getload.h"
#include "../config.h"

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_LOADAVG_H
#  include <loadavg.h>
#endif
#ifdef HAVE_SYS_LOADAVG_H
#  include <sys/loadavg.h>
#endif

double GetLoad()
{
   double avg = 0.0;

#ifdef HAVE_GETLOADAVG
   getloadavg(&avg, 1);
#else
#  error "don't know how to get the load average"
#endif

   return avg;
}
