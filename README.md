JLoad
==============================================================================
System load average display for JWM.  This program is intended to be used
in JWM trays via the "Swallow" tag, though it can be used without JWM as
a substitute for xload.

Unlike xload, JLoad displays a normalized load average (the load average
divided by the number of CPU cores).  In addition, it allows one to
specify a command to execute when clicked.

Building
------------------------------------------------------------------------------
Run `make` and copy the resulting `jload` application somewhere in your $PATH.

Usage
------------------------------------------------------------------------------
```
jload [options]
  -command <string>   Command to run when clicked
  -display <string>   Set the display to use
  -geometry <string>  Window geometry
  -fg <string>        Foreground color
  -bg <string>        Background color
  -hl <string>        Scale color
```

Using JLoad with JWM
------------------------------------------------------------------------------ 
To use JLoad within a tray in JWM, add the following to your ~/.jwmrc:
```xml
<Tray>
   <!-- ... -->
   <Swallow name="jload">jload</Swallow>
   <!-- ... -->
</Tray>
```
