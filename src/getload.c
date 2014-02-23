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
#ifdef HAVE_SYS_SYSGET_H
#  include <sys/sysget.h>
#endif

double GetLoad()
{
   double avg = 0.0;

#ifdef HAVE_GETLOADAVG
   getloadavg(&avg, 1);
#elif defined(HAVE_SYSGET)
   struct sgt_cookie cookie;
   int load[3];
   SGT_COOKIE_INIT(&cookie);
   SGT_COOKIE_SET_KSYM(&cookie, KSYM_AVENRUN);
   sysget(SGT_KSYM, (char*)&load, sizeof(load), SGT_READ, &cookie);
   avg = (double)load[0] / 1024.0;
   printf("%g\n", (float)avg);
#else
#  error "don't know how to get the load average"
#endif

   return avg;
}
