#include "graphics.h"
#include <string.h>

int i, j = 0, gd = DETECT, gm;

//Limited to between 0 and 99999 - Height/20
static int start_num = 0;
static int counter = 0;
static int cursor_col = BLACK;
static bool cursor_down = false;

const int Width = 800;
const int Height = 600;
const char left[5] = {'L','e','f','t','\0'};
const char right[6] = {'R','i','g','h','t','\0'};
const char up[3] = {'U','p','\0'};
const char down[5] = {'D','o','w','n','\0'};

static char buf[10000000] = "";

struct Point
{
    int x;
    int y;
    Point() {}
    Point(int a, int b)
    {
        x = a;
        y = b;
    }
};

static Point cursor = {0,0};
static Point mouse_start = {0,0};


template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

void drawAxisY(int offSet, int color)
{
    for (int i = 0; i < Width; i++)
    {
        putpixel(i, Height - offSet - 20, color);
    }
}

void drawAxisX(int offSet, int color)
{
    for (int i = 0; i < Height; i++)
    {
        putpixel(offSet, i, color);
    }
}

void drawFillRect(Point a, Point b, int col)
{
    int old_col = getcolor();
    setcolor(RED);
    rectangle(a.x,a.y,b.x,b.y);
    setfillstyle(SOLID_FILL,col);
    floodfill(a.x+1,a.y+1,RED);
    setcolor(col);
    rectangle(a.x,a.y,b.x,b.y);
    setcolor(old_col);
}

void drawText(Point a, char* text) 
{
    int old_col = getcolor();
    int old_bk_col = getbkcolor();
    setbkcolor(getpixel(a.x,a.y));
    setcolor(BLACK);
    outtextxy(a.x,a.y,text);
    setcolor(old_col);
    setbkcolor(getbkcolor());
}

//Returns the text at the cursor position
int getTextPosition()
{
    int pos = 0;
    int row = 0;
    int col = 0;
    while (row < cursor.y)
    {
        if (buf[pos] == '\n')
        {
            row++;
        }
        pos++;
    }
    while(col < cursor.x)
    {
        if(buf[pos] == '\n' || buf[pos] == '\0')
        {
            break;
        }
        pos++;
        col++;
    }
    return pos;
}

//Returns the actual cursor position 
Point getActualCursor()
{
    int pos = 0;
    int row = 0;
    int col = 0;
    while (row < cursor.y)
    {
        if (buf[pos] == '\n')
        {
            row++;
        }
        if(buf[pos] == '\0')
        {
            return {0,row};
        }
        pos++;
    }
    while(col < cursor.x)
    {
        if(buf[pos] == '\n' || buf[pos] == '\0')
        {
            break;
        }
        pos++;
        col++;
    }
    return {col, row};
}

void drawCursor(int col)
{
    int old_col = getcolor();
    Point actual_cursor = getActualCursor();
    setcolor(col);
    line(61+12*actual_cursor.x,(actual_cursor.y - start_num)*20,61+12*actual_cursor.x,20+(actual_cursor.y - start_num)*20);
    setcolor(old_col);
}

//Clears the rest of the row after the position
void clearRowAfter(Point cursor, int position)
{
    int old_col = getbkcolor();
    setbkcolor(WHITE);
    Point cur = cursor;
    if (cur.x > 0)
    {
        cur.x -= 1;
    }
    int pos = position-1;
    while(buf[pos] != '\0' && buf[pos] != '\n')
    {
        char str[2] = {' ','\0'};
        drawText({62 + cur.x*12, cur.y*20}, str);
        cur = {cur.x+1,cur.y};
        pos++;
    }
    setbkcolor(old_col);
}

//Clears all text after the position
void clearAllAfter(Point cursor, int position)
{
    int old_col = getbkcolor();
    setbkcolor(WHITE);
    Point cur = cursor;
    int pos = position;
    while(buf[pos] != '\0')
    {
        if (buf[pos] == '\n')
        {
            cur = {0, cur.y+1};
        }
        else
        {
            char str[2] = {' ','\0'};
            drawText({62 + cur.x*12, cur.y*20}, str);
            cur = {cur.x+1,cur.y};
        }
        pos++;
    }
    setbkcolor(old_col);
}

void addCharAtCursor(char *str)
{
    //Gets the position to add it into
    int pos = getTextPosition();

    //Clear all written strings after this
    if(str[0] == '\n')
    {
        clearAllAfter(getActualCursor(), pos);
        cursor = {0, cursor.y+1};
    }
    else 
    {
        clearRowAfter(getActualCursor(), pos);
        cursor = {cursor.x+1,cursor.y};
    }

    //Adds the new char in and moves all other chars 
    for(int i = strlen(buf); i >= pos; i--)
    {
        buf[i+1] = buf[i];
    }
    buf[pos] = str[0];
}

void removeCharAtCursor()
{
    //Gets the position to add it into
    int pos = getTextPosition();

    //Clear all written strings after this
    if(buf[pos-1] == '\n')
    {
        clearAllAfter(getActualCursor(), pos);
        cursor = {9999, cursor.y-1};
        cursor = getActualCursor();
    }
    else 
    {
        clearRowAfter(getActualCursor(), pos);
        cursor = {cursor.x-1,cursor.y};
    }

    for(int i = pos; i <= strlen(buf); i++)
    {
        buf[i-1] = buf[i];
    }
}

void drawText()
{
    int old_col = getbkcolor();
    setbkcolor(WHITE);
    int row = 0;
    int col = 0;
    int pos = 0;
    while (buf[pos] != '\0') {
        if (buf[pos] == '\n')
        {
            row++;
            col = 0;
        }
        else 
        {
            if(row >= start_num && row < start_num + Height/20)
            {
                char str[2] = {buf[pos],'\0'};
                drawText({62+col*12,(row-start_num)*20}, str);
                col++;
            }
        }
        pos++;
    }
    setbkcolor(old_col);
}

void drawSidebar()
{
    int old_col = getbkcolor();
    setbkcolor(LIGHTGRAY);
    for(int i = 0; i < 30; i++)
    {
        int num = start_num + i;
        char str[10] = "";
        char strnum[10];
        char str2[] = "0";
        itoa(num,strnum,10);
        for(int j = 0; j < 5 - strlen(strnum); j++)
        {
            strcat(str,str2);
        }
        strcat(str,strnum);
        drawText({0,i*20},str);
    }
    setbkcolor(old_col);
}

int main()
{
    initwindow(Width, Height);
    buf[0] = '\0';
    settextjustify(LEFT_TEXT,TOP_TEXT);
    settextstyle(DEFAULT_FONT, 0, 0);
    setbkcolor(WHITE);
    cleardevice( );
    drawFillRect({0,0},{60,Height},LIGHTGRAY);
    drawSidebar( );
    while (true) {
        if (ismouseclick(WM_MOUSEWHEEL))
        {
            int scroll_num = getmousescroll();
            if (scroll_num != 0 && start_num - sgn(scroll_num) >= 0)
            {
                clearAllAfter({0,0},0);
                drawCursor(WHITE);
                start_num -= sgn(scroll_num);
                drawText( );
                drawCursor(BLACK);
                drawSidebar( );
            }
        }
        if(ismouseclick(WM_LBUTTONUP) && cursor_down)
        {
            if (abs(mouse_start.x - mousex()) + abs(mouse_start.y - mousey()) < 2)
            {
                drawCursor(WHITE);
                cursor = {(mousex() - 60)/12, mousey()/20};
                drawCursor(BLACK);
            }
            else
            {
                //Highlight a blue region
            }
            cursor_down = false;
        }
        else if(ismouseclick(WM_LBUTTONDOWN) && !cursor_down)
        {
            mouse_start = {mousex(),mousey()};
            cursor_down = true;
        }
        if (isKeypress())
        {
            drawCursor(WHITE);
            if (checkPressed("Backspace")) 
            {
                if (!(cursor.x == 0 and cursor.y == 0))
                {
                    removeCharAtCursor();
                }
            }
            else if (checkPressed(left)) 
            {
                if (cursor.x > 0)
                {
                    Point actual_cursor = getActualCursor();
                    cursor = {actual_cursor.x-1,cursor.y};
                }
                else if (cursor.x == 0 and cursor.y > 0)
                {
                    cursor = {9999,cursor.y-1};
                    cursor = getActualCursor();
                }
            }
            else if (checkPressed(right))
            {
                Point actual_cursor = getActualCursor();
                char c = buf[getTextPosition()];
                if (c != '\n' && c != '\0')
                {
                    cursor = {actual_cursor.x+1,cursor.y};
                }
                else if (c == '\n')
                {
                    cursor = {0, cursor.y + 1};
                }
            }
            else if (checkPressed(down))
            {
                cursor = {cursor.x, cursor.y+1};
                Point actual_cursor = getActualCursor();
                if (cursor.y == actual_cursor.y)
                {
                    cursor = {cursor.x, cursor.y};
                }
                else
                {
                    cursor = {cursor.x, cursor.y-1};
                }
            }
            else if (checkPressed(up))
            {
                if (cursor.y > 0)
                {
                    cursor = {cursor.x, cursor.y-1};
                }
            }
            else
            {
                char *str = (char *)malloc(sizeof(char)*50);
                str[0] = '\0';
                keypressName(str);
                if (str[0] != '\0') 
                {
                    addCharAtCursor(str);
                }
                free(str);
            }
            drawCursor(BLACK);
            drawText();
        }

    
        if (counter % 50 == 0) {
            cursor_col = BLACK;
            drawCursor(cursor_col);
        }
        else if (counter % 25 == 0) {
            cursor_col = WHITE;
            drawCursor(cursor_col);
        }
        counter++;
        delay(1);
    }
    getchar();
    return 0;
}