#include "cpucount.h"
#include "../config.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#  include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_MPCTL_H
#  include <sys/mpctl.h>
#endif

/* Determine which method to use for finding the CPU count. */
#if defined(HAVE_SYSCONF) && defined(_SC_NPROCESSORS_ONLN)
#  define USE_SC_NPROCESSORS
#elif defined(HAVE_SYSCONF) && defined(_SC_NPROC_ONLN)
#  define USE_SC_NPROC
#elif defined(HAVE_MPCTL) && defined(MPC_GETNUMSPUS)
#  define USE_MPC
#else
#  warning "Don't know how to get the number of CPUs on this platform"
#endif

unsigned int GetCPUCount()
{
   static unsigned int cpu_count = 0;

#ifdef USE_SC_NPROCESSORS
   if(cpu_count == 0) {
      cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
      cpu_count = cpu_count == 0 ? 1 : cpu_count;
   }
#endif

#ifdef USE_SC_NPROC
   if(cpu_count == 0) {
      cpu_count = sysconf(_SC_NPROC_ONLN);
      cpu_count = cpu_count == 0 ? 1 : cpu_count;
   }
#endif

#ifdef USE_SYSCTL
   if(cpu_count == 0) {
      int mib[4];
      const size_t len = sizeof(cpu_count);
      mib[0] = CTL_HW;
      mib[1] = HW_AVAILCPU;
      sysctl(mib, 2, &cpu_count, &len, NULL, 0);
      if(cpu_count < 1) {
         mib[1] = HW_NCPU;
         sysctl(mib, 2, &cpu_count, &len, NULL, 0);
         cpu_count = cpu_count == 0 ? 1 : cpu_count;
      }
   }
#endif

#ifdef USE_MPC
   if(cpu_count == 0) {
      cpu_count = mpctl(MPC_GETNUMSPUS, NULL, NULL);
   }
#endif

   /* Assume a single CPU if we still don't have a value. */
   if(cpu_count == 0) {
      cpu_count = 1;
   }
   return cpu_count;
}
