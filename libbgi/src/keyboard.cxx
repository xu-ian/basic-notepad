#include <windows.h>        // Provides Win32 API
#include <windowsx.h>       // Provides GDI helper macros
#include "../include/bgi/winbgim.h"         // API routines
#include "../include/bgi/winbgitypes.h"    // Internal structure data

/* Helper function to change value of keypresses when shift is pressed.
 * Input is a char string of length 1, should replace the string based on whether shift is pressed
 */
void shiftConversion(char *str)
{
  WindowData *pWndData = BGI__GetWindowDataPtr( );

  if (pWndData->key_down[VK_SHIFT] || pWndData->key_down[VK_LSHIFT] || pWndData->key_down[VK_RSHIFT])
  {
    if(str[0] == '1') {
      str[0] = '!';
    }
    if(str[0] == '2') {
      str[0] = '@';
    }
    if(str[0] == '3') {
      str[0] = '#';
    }
    if(str[0] == '4') {
      str[0] = '$';
    }
    if(str[0] == '5') {
      str[0] = '%';
    }
    if(str[0] == '6') {
      str[0] = '^';
    }
    if(str[0] == '7') {
      str[0] = '&';
    }
    if(str[0] == '8') {
      str[0] = '*';
    }
    if(str[0] == '9') {
      str[0] = '(';
    }
    if(str[0] == '0') {
      str[0] = ')';
    }
    if(str[0] == '`') 
    {
      str[0] = '~';
    }
    for(int i = 0; i < 3; i++)
    {
      if (str[0] == 91 + i)
      {
        str[0] += 32;
      }
    }
    if(str[0] == '-')
    {
      str[0] = '_';
    }
    if(str[0] == '=')
    {
      str[0] = '+';
    }
    if(str[0] == ';')
    {
      str[0] = ':';
    }
    if(str[0] == '\'')
    {
      str[0] = '"';
    }
  }
  else
  {
    if (str[0] >=65 && str[0] <= 90) 
    {
      str[0] += 32;
    }
  }
}

//Check if any key is pressed
bool isKeypress()
{
  WindowData *pWndData = BGI__GetWindowDataPtr( );

  for(int i = 0; i < 256; i++)
  {
    if(pWndData->key_down[i] && i != VK_SHIFT && i != VK_CONTROL)
    {
      return true;
    }
  }

  return false;
}

bool checkKeypress(char c)
{
  WindowData *pWndData = BGI__GetWindowDataPtr( );
  return pWndData->key_down[c];
}

//Check if a given special key is pressed
bool checkPressed(const char *key)
{
  WindowData *pWndData = BGI__GetWindowDataPtr( );
  bool tmp = false;
  if (strcmp(key,"Left") == 0)
  {
    tmp = pWndData->key_down[VK_LEFT];
    pWndData->key_down[VK_LEFT] = false;
    return tmp;
  }
  else if (strcmp(key,"Right") == 0) 
  {
    tmp = pWndData->key_down[VK_RIGHT];
    pWndData->key_down[VK_RIGHT] = false;
    return tmp;
  }
  else if (strcmp(key,"Up") == 0)
  {
    tmp = pWndData->key_down[VK_UP];
    pWndData->key_down[VK_UP] = false;
    return tmp;
  }
  else if (strcmp(key,"Down") == 0)
  {
    tmp = pWndData->key_down[VK_DOWN];
    pWndData->key_down[VK_DOWN] = false;
    return tmp;
  }
  else if (strcmp(key,"Backspace") == 0)
  {
    tmp = pWndData->key_down[VK_BACK];
    pWndData->key_down[VK_BACK] = false;
    return tmp;
  }
  else if (strcmp(key,"Shift") == 0)
  {
    return pWndData->key_down[VK_SHIFT];
  }
  else if (strcmp(key,"Control") == 0)
  {
    return pWndData->key_down[VK_CONTROL];
  }
  return false;
}

//Get the character associated with the keypress
void keypressName(char *str)
{
  WindowData *pWndData = BGI__GetWindowDataPtr( );

  for(int i = 0; i < 256; i++)
  {
    if(pWndData->key_down[i])
    {
      int len = GetKeyNameTextA(pWndData->key_value[i],str,sizeof(str));
      if (len == 1) {
        shiftConversion(str);
        pWndData->key_down[i] = false;
        return;
      }
      else if (strcmp(str,"Space") == 0) 
      {
        str[0] = char(32);
        str[1] = '\0';
        pWndData->key_down[VK_SPACE] = false;
        return;
      }
      else if (strcmp(str,"Enter") == 0)
      {
        str[0] = '\n';
        str[1] = '\0';
        pWndData->key_down[VK_RETURN] = false;
        return;
      }
      else if (strcmp(str,"Backspa") == 0)
      {
        str[0] = char(8);
        str[1] = '\0';
        pWndData->key_down[VK_BACK] = false;
        return;
      }
    }

  }
  str[0] = '\0';
}