#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "../config.h"
#include "getload.h"
#include "cpucount.h"

#define VERSION               "0.1"
#define DEFAULT_WIDTH         32
#define DEFAULT_HEIGHT        32
#define DEFAULT_BACKGROUND    0x000000
#define DEFAULT_FOREGROUND    0xFF0000
#define DEFAULT_HL            0xFFFFFF
#define DEFAULT_INTERVAL_MS   10000
#define SHELL_NAME            "/bin/sh"

static Atom delete_atom;
static Atom protocols_atom;
static unsigned int width = DEFAULT_WIDTH;
static unsigned int height = DEFAULT_HEIGHT;
static long interval_ms = DEFAULT_INTERVAL_MS;
static unsigned long fg = DEFAULT_FOREGROUND;
static unsigned long hl = DEFAULT_HL;
static Display *display;
static Window window;
static GC gc;
static double *averages = NULL;
static const char *command = NULL;

/** Run the command. */
static void RunCommand()
{
   if(command) {
      if(!fork()) {
         close(ConnectionNumber(display));
         setsid();
         execl(SHELL_NAME, SHELL_NAME, "-c", command, NULL);
         exit(EXIT_SUCCESS);
      }
   }
}

/** Update the load chart. */
static void UpdateLoad()
{
   const double load = GetLoad();
   const unsigned int cpu_count = GetCPUCount();
   if(!averages) {
      averages = (double*)calloc(width, sizeof(double));
   }
   memmove(&averages[1], averages, sizeof(double) * (width - 1));
   averages[0] = load / cpu_count;
}

/** Draw the window. */
static void DrawWindow()
{

   static struct timeval last_update = { 0, 0 };
   struct timeval tv;
   long diff_ms;
   int i;
   int scale;

   /* Determine if we should update the display. */
   gettimeofday(&tv, NULL);
   diff_ms = (tv.tv_sec - last_update.tv_sec) * 1000L;
   diff_ms += (tv.tv_usec - last_update.tv_usec) / 1000L;
   if(diff_ms >= interval_ms || last_update.tv_sec == 0) {
      UpdateLoad();
      last_update = tv;
   }

   /* Determine the scale. */
   scale = 1;
   for(i = 0; i < width; i++) {
      if(averages[i] >= scale - 1) {
         scale = (int)averages[i] + 1;
      }
   }

   /* Clear the area. */
   XClearWindow(display, window);

   /* Draw the load graph. */
   XSetForeground(display, gc, fg);
   for(i = 0; i < width; i++) {
      const int x = width - i;
      const int y = height - (int)(height * (averages[i] / (double)scale));
      if(y < height) {
         XDrawLine(display, window, gc, x, height, x, y);
      }
   }

   /* Draw bars. */
   XSetForeground(display, gc, hl);
   for(i = 1; i < scale; i++) {
      const int y = i * (height / scale);
      XDrawLine(display, window, gc, 0, y, width, y);
   }

}

/** Handle a configure event. */
static void HandleConfigure(const XConfigureEvent *event)
{
   if(event->width > width) {
      int i;
      averages = (double*)realloc(averages, sizeof(double) * event->width);
      for(i = width; i < event->width; i++) {
         averages[i] = 0.0;
      }
   }
   width = event->width;
   height = event->height;
}

/** Process events. */
static void EventLoop()
{
   int fd = ConnectionNumber(display);

   /* Loop processing events. */
   for(;;) {
      XEvent event;

      /* Wait for an event. */
      while(XPending(display) == 0) {
         struct timeval timeout;
         fd_set fds;

         FD_ZERO(&fds);
         FD_SET(fd, &fds);

         timeout.tv_sec = interval_ms / 1000;
         timeout.tv_usec = (interval_ms % 1000) * 1000;
         if(select(fd + 1, &fds, NULL, NULL, &timeout) <= 0) {
            DrawWindow();
         }
      }


      XNextEvent(display, &event);
      switch(event.type) {
      case ConfigureNotify:
         HandleConfigure(&event.xconfigure);
         break;
      case ClientMessage:
         if(event.xclient.message_type == protocols_atom &&
            event.xclient.data.l[0] == delete_atom) {
            return;
         }
         break;
      case Expose:
         DrawWindow();
         break;
      case ButtonPress:
         RunCommand();
         break;
      default:
         break;
      }
   }
}

/** Display usage. */
static void DisplayHelp(const char *name)
{
   printf("JLoad v" PACKAGE_VERSION " by Joe Wingbermuehle <"
          PACKAGE_BUGREPORT ">\n");
   printf("usage: %s [options]\n", name);
   printf("  -command <string>   Command to run when clicked\n");
   printf("  -display <string>   Set the display to use\n");
   printf("  -geometry <string>  Window geometry\n");
   printf("  -fg <string>        Foreground color\n");
   printf("  -bg <string>        Background color\n");
   printf("  -hl <string>        Scale color\n");
}

/** Look up a color by name. */
static long GetColor(const char *name, long def)
{
   if(name) {
      XColor color;
      Colormap cmap = DefaultColormap(display, DefaultScreen(display));
      if(XParseColor(display, cmap, name, &color)) {
         XAllocColor(display, cmap, &color);
         return color.pixel;
      } else {
         fprintf(stderr, "WARN: color not found: %s\n", name);
      }
   }
   return def;
}

/** The main function. */
int main(int argc, char *argv[])
{
   XClassHint *chint;
   const char *display_string = NULL;
   const char *fg_string = NULL;
   const char *bg_string = NULL;
   const char *hl_string = NULL;
   unsigned long bg = DEFAULT_BACKGROUND;
   Window root;
   int x, y;
   int i;

   /* Parse arguments. */
   for(i = 1; i < argc; i++) {
      if(!strcmp(argv[i], "-command") && i + 1 < argc) {
         command = argv[i + 1];
         i += 1;
      } else if(!strcmp(argv[i], "-display") && i + 1 < argc) {
         display_string = argv[i + 1];
         i += 1;
      } else if(!strcmp(argv[i], "-geometry") && i + 1 < argc) {
         XParseGeometry(argv[i + 1], &x, &y, &width, &height);
         i += 1;
      } else if(!strcmp(argv[i], "-fg") && i + 1 < argc) {
         fg_string = argv[i + 1];
         i += 1;
      } else if(!strcmp(argv[i], "-bg") && i + 1 < argc) {
         bg_string = argv[i + 1];
         i += 1;
      } else if(!strcmp(argv[i], "-hl") && i + 1 < argc) {
         hl_string = argv[i + 1];
         i += 1;
      } else if(!strcmp(argv[i], "-interval") && i + 1 < argc) {
         interval_ms = atoi(argv[i + 1]);
         if(interval_ms < 1) {
            fprintf(stderr, "WARN: invalid -interval: %s\n", argv[i + 1]);
            interval_ms = DEFAULT_INTERVAL_MS;
         }
         i += 1;
      } else {
         DisplayHelp(argv[0]);
         return -1;
      }
   }

   /* Open the display. */
   display = XOpenDisplay(display_string);
   if(!display) {
      fprintf(stderr, "ERROR: could not open display\n");
      return -1;
   }

   fg = GetColor(fg_string, DEFAULT_FOREGROUND);
   bg = GetColor(bg_string, DEFAULT_BACKGROUND);
   hl = GetColor(hl_string, DEFAULT_HL);

   /* Create the window. */
   root = XDefaultRootWindow(display);
   window = XCreateSimpleWindow(display, root, 0, 0, width, height, 0, 0, bg);
   XStoreName(display, window, "JLoad");
   chint = XAllocClassHint();
   chint->res_name = "jload";
   chint->res_class = "JLoad";
   XSetClassHint(display, window, chint);
   XFree(chint);

   /* Inform the WM that we can be deleted. */
   delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
   protocols_atom = XInternAtom(display, "WM_PROTOCOLS", False);
   XChangeProperty(display, window, protocols_atom, XA_ATOM, 32,
                   PropModeReplace, (unsigned char*)&delete_atom, 1);

   /* Select events we care about and map. */
   XSelectInput(display, window,
                ExposureMask | StructureNotifyMask | ButtonPressMask);
   XMapWindow(display, window);

   /* Create the graphics context for drawing. */
   gc = XCreateGC(display, window, 0, NULL);

   /* Process events until it's time to exit. */
   EventLoop();

   /* Clean up. */
   XFreeGC(display, gc);
   XDestroyWindow(display, window);
   XCloseDisplay(display);
   return 0;

}
