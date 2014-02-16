
#include <stdlib.h>

double GetLoad()
{
   double avg;
   getloadavg(&avg, 1);
   return avg;
}
