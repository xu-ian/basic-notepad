#include "graphics.h"
#include <string.h>

int i, j = 0, gd = DETECT, gm;

//Limited to between 0 and 99999 - Height/20
static int start_num = 0;
static int counter = 0;

const int Width = 800;
const int Height = 600;

static char buffer[1000000] = "";

struct Point
{
    double x;
    double y;
    Point() {}
    Point(double a, double b)
    {
        x = a;
        y = b;
    }
};

static Point cursor = {0,0};

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
    setcolor(col);
    setfillstyle(SOLID_FILL,col);
    rectangle(a.x,a.y,b.x,b.y);
    floodfill(a.x+1,a.y+1,col);
    setcolor(old_col);
}

void drawText(Point a, char* text) 
{
    int old_bk_col = getbkcolor();
    int old_col = getcolor();
    setcolor(BLACK);
    setbkcolor(getpixel(a.x,a.y));
    outtextxy(a.x,a.y,text);
    setcolor(old_col);
    setbkcolor(old_bk_col);
}

void drawCursor(int col)
{
    int old_col = getcolor();
    setcolor(col);
    line(61+12*cursor.x,(cursor.y - start_num)*20,61+12*cursor.x,20+(cursor.y - start_num)*20);
    setcolor(old_col);
}

void drawSidebar()
{
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
}

int main()
{
    initwindow(Width, Height);
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
            if (scroll_num != 0 && start_num - scroll_num/120 >= 0)
            {
                drawCursor(WHITE);
                start_num -= scroll_num/120;
                drawCursor(BLACK);
                drawSidebar( );
            }
        }
        if (counter % 50 == 0) {
            drawCursor(BLACK);
        }
        else if (counter % 25 == 0) {
            drawCursor(WHITE);
        }
        counter++;
        delay(10);
    }
    getchar();
    return 0;
}