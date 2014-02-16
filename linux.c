
#include <stdlib.h>
#include <unistd.h>

double GetLoad()
{
   static long cpu_count = 0;
   double avg;

   /* Get the number of CPUs. */
   if(cpu_count == 0) {
      cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
      cpu_count = cpu_count == 0 ? 1 : cpu_count;
   }

   /* Get the total load average. */
   getloadavg(&avg, 1);

   /* Return the normalized load average. */
   return avg / (double)cpu_count;

}
