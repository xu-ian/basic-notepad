#include "graphics.h"
#include <string.h>

int i, j = 0, gd = DETECT, gm;

//Limited to between 0 and 99999 - Height/20
static int start_num = 0;
static int counter = 0;
static int cursor_col = BLACK;
static bool cursor_down = false;

const int Width = 800;
const int Height = 624;
const int xoffset = 62;
const int yoffset = 24;
const int charwidth = 12;
const int charheight = 20;

const char left[5] = {'L','e','f','t','\0'};
const char right[6] = {'R','i','g','h','t','\0'};
const char up[3] = {'U','p','\0'};
const char down[5] = {'D','o','w','n','\0'};

static char buf[10000000] = "";

static char clipboard[1000] = "";

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
static Point copy_start = {-1,-1};

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
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

Point pixelToCoordinate(Point a)
{
    return Point(xoffset+a.x*charwidth,yoffset+(a.y-start_num)*charheight);
}

Point coordinateToPixel(Point a)
{
    return Point((a.x-xoffset)/charwidth,(a.y-yoffset)/charheight);
}

void drawText(Point a, char* text) 
{
    if(a.y == yoffset)
    {
        printf("%s\n", text);
    }
    int old_col = getcolor();
    int old_bk_col = getbkcolor();
    (getpixel(a.x,a.y) == LIGHTGRAY) 
    ? setbkcolor(LIGHTGRAY) 
    : (getpixel(a.x,a.y) == DARKGRAY)
      ? setbkcolor(DARKGRAY)
      : setbkcolor(WHITE);
    setcolor(BLACK);
    outtextxy(a.x,a.y,text);
    setcolor(old_col);
    setbkcolor(getbkcolor());
}

//Returns the text at the cursor position
int getTextPosition(Point cur)
{
    int pos = 0, row = 0, col = 0;
    while (row < cur.y || col < cur.x)
    {
        if (buf[pos] == '\0')
        {
            return pos;
        }
        col++;
        if (buf[pos] == '\n')
        {
            row++;
            col = 0;
        }
        pos++;
    }
    return pos;
}

//Returns the actual cursor position 
Point getActualCursor(Point cur)
{
    int pos = 0, row = 0, col = 0;
    while (row < cur.y || col < cur.x)
    {
        if(buf[pos] == '\0') { return {col,row}; }

        if (buf[pos] == '\n')
        {
            if (row == cur.y) { return {col, row}; }
            row++;
            col = 0;
        }
        else
        {
            col++;
        }
        pos++;
    }
    return {col, row};
}

int getFirstVisiblePosition()
{
    int pos = 0;
    int row = 0;
    while(buf[pos] != '\0' && row < start_num)
    {
        if (buf[pos] == '\n')
        {
            row++;
        }
        pos++;
    }
    return pos;
}

void drawCursor(int col)
{
    if (cursor.x < 0 || cursor.y < start_num) { return; }
    int old_col = getcolor();
    Point cur = getActualCursor(cursor);
    Point bgn = pixelToCoordinate(cur), dst = pixelToCoordinate(Point(cur.x,cur.y+1));
    setcolor(col);
    line(bgn.x,bgn.y,dst.x,dst.y);
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
        drawText(pixelToCoordinate(cur), str);
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
            drawText(pixelToCoordinate(cur), str);
            cur = {cur.x+1,cur.y};
        }
        pos++;
    }
    setbkcolor(old_col);
}

void addCharAtCursor(char *str)
{
    //Gets the position to add it into
    int pos = getTextPosition(cursor);

    //Clear all written strings after this
    if (str[0] == '\n')
    {
        clearAllAfter(getActualCursor(cursor), pos);
        cursor = {0, cursor.y+1};
    }
    else 
    {
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
    int pos = getTextPosition(cursor);

    //Clear all written strings after this
    if(buf[pos-1] == '\n')
    {
        clearAllAfter(getActualCursor(cursor), pos);
        cursor = getActualCursor({9999, cursor.y-1});
    }
    else 
    {
        clearRowAfter(getActualCursor(cursor), pos);
        cursor = {cursor.x-1,cursor.y};
    }

    for(int i = pos; i <= strlen(buf); i++)
    {
        buf[i-1] = buf[i];
    }
}

void drawText()
{
    int row = 0, col = 0, pos = 0;
    while (buf[pos] != '\0') {
        if (buf[pos] == '\n')
        {
            row++;
            col = 0;
        }
        else if (row >= start_num && row < start_num + Height/20)
        {
            char str[2] = {buf[pos],'\0'};
            drawText(pixelToCoordinate({col,row}), str);
            col++;
        }
        pos++;
    }
}

void drawPosition()
{
    char str[5];
    itoa(cursor.y,str,10);
    drawText({50*12,0},"Row: ");
    drawText({60*12,0},"Col: ");
    drawText({54*12,0},"     ");
    drawText({54*12,0},str);
    itoa(cursor.x,str,10);
    drawText({64*12,0},"     ");
    drawText({64*12,0},str);
}

void drawSidebar()
{
    for(int i = 0; i < 30; i++)
    {
        int num = start_num + i;
        char str[10] = "", strnum[10];
        itoa(num,strnum,10);
        for(int j = 0; j < 5 - strlen(strnum); j++)
        {
            strcat(str,"0");
        }
        strcat(str,strnum);
        drawText({0,yoffset + i*charheight},str);
    }
}

void checkScroll()
{
    if (cursor.y < start_num || cursor.y >= start_num + 30)
    {
        clearAllAfter({0,0},getFirstVisiblePosition());
        start_num += (cursor.y < start_num) ? -1 : 1;
        drawText();
        drawSidebar();
    }
}

void highlightSelection(int col)
{
    if (copy_start.x != -1)
    {
        Point temp = {cursor.x, cursor.y};
        int pos1 = getTextPosition(cursor), pos2 = getTextPosition(copy_start);
        if (pos1 == pos2)
        {
            return;
        }
        else if (pos1 > pos2)
        {
            int tmp = pos1;
            pos1 = pos2;
            pos2 = tmp;
            temp = {copy_start.x,copy_start.y};
        }
        int old_col = getcolor();
        setcolor(col);
        //Underline all chars 
        for(int i = pos1; i < pos2; i++)
        {
            if (buf[i] != '\n')
            {
                Point bgn = pixelToCoordinate({temp.x,temp.y+1}),dst = pixelToCoordinate({temp.x+1,temp.y+1});
                line(bgn.x,bgn.y,dst.x,dst.y);
            }
            temp = (buf[i] == '\n') ? Point(0, temp.y+1) : Point(temp.x+1, temp.y);
        }
        setcolor(old_col);
    }
}

void trackCopy()
{
    copy_start = (!checkPressed("Shift")) ? Point(-1,-1) : copy_start ;
}

void copySelection()
{
    if (copy_start.x != -1)
    {
        int pos1 = getTextPosition(cursor), pos2 = getTextPosition(copy_start);
        if (pos1 == pos2)
        {
            return;
        }
        else if (pos1 > pos2)
        {
            int tmp = pos1;
            pos1 = pos2;
            pos2 = tmp;
        }
        for(int i = pos1; i < pos2; i++)
        {
            clipboard[i-pos1] = buf[i];
        }
        clipboard[pos2] = '\0';
        //printf("Copied selection:%s\n", clipboard);
    }
}

void clearSelection()
{
    Point temp = {cursor.x, cursor.y};
    int pos1 = getTextPosition(cursor), pos2 = getTextPosition(copy_start);

    if (pos1 < pos2)
    {
        int tmp = pos1;
        pos1 = pos2;
        pos2 = tmp;
        cursor = {copy_start.x,copy_start.y};    
    }

    while(pos1 > pos2)
    {
        removeCharAtCursor();
        pos1--;
    }
    cursor = {temp.x,temp.y};
    copy_start = {-1,-1};
}

void pasteSelection()
{
    if (clipboard[0] == '\0') { return; }
    int acc = 0;
    while(clipboard[acc] != '\0')
    {  
        char str[2] = {clipboard[acc],'\0'};
        addCharAtCursor(str);
        acc++;
    }
    copy_start = {-1,-1};
    drawText();
}

int main()
{
    initwindow(Width, Height);
    settextstyle(DEFAULT_FONT, 0, 0);
    setbkcolor(WHITE);
    cleardevice();
    drawFillRect({0,0},{Width,yoffset-1},DARKGRAY);
    drawFillRect({0,yoffset},{xoffset-2,Height},LIGHTGRAY);
    drawPosition();
    drawSidebar();
    while (true) {
        if (ismouseclick(WM_MOUSEWHEEL))
        {
            int scroll_num = getmousescroll();
            if (scroll_num != 0 && start_num - sgn(scroll_num) >= 0)
            {
                clearAllAfter({0,0},getFirstVisiblePosition());
                drawCursor(WHITE);
                start_num -= sgn(scroll_num);
                drawText( );
                drawCursor(BLACK);
                drawSidebar( );
            }
        }
        if(ismouseclick(WM_LBUTTONUP))
        {
            drawCursor(WHITE);
            cursor = coordinateToPixel({mousex(),mousey()});
            drawCursor(BLACK);
            clearmouseclick(WM_LBUTTONUP);
            clearmouseclick(WM_LBUTTONDOWN);
            cursor_down = false;
            drawPosition();
        }
        else if(ismouseclick(WM_LBUTTONDOWN) && !cursor_down)
        {
            highlightSelection(WHITE);
            copy_start = coordinateToPixel({mousex(),mousey()});
            cursor_down = true;
            drawPosition();
        }
        else if(ismouseclick(WM_MOUSEMOVE) && ismouseclick(WM_LBUTTONDOWN))
        {
            drawCursor(WHITE);
            cursor = coordinateToPixel({mousex(),mousey()});
            highlightSelection(BLACK);
            drawCursor(BLACK);
            clearmouseclick(WM_MOUSEMOVE);
        }
        
        if (checkPressed("Shift") && copy_start.x == -1){
            copy_start = {cursor.x, cursor.y};
            printf("Starting at (%d,%d)\n", copy_start.x,copy_start.y);
        }

        if (isKeypress())
        {
            drawCursor(WHITE);
            highlightSelection(WHITE);
            if (checkPressed("Backspace")) 
            {
                copy_start = {-1,-1};
                if (!(cursor.x == 0 and cursor.y == 0))
                {
                    removeCharAtCursor();
                    drawText();
                }
            }
            else if (checkPressed(left)) 
            {
                cursor = (cursor.x > 0) 
                         ? Point(getActualCursor(cursor).x-1,cursor.y) 
                         : (cursor.x == 0 and cursor.y > 0)
                           ? getActualCursor({9999,cursor.y-1})
                           : cursor ;
            }
            else if (checkPressed(right))
            {
                char c = buf[getTextPosition(cursor)];
                cursor = (c == '\n')
                         ? Point(0, cursor.y+1)
                         : (c == '\0')
                           ? cursor
                           : Point(cursor.x+1,cursor.y);
            }
            else if (checkPressed(down))
            {
                cursor = getActualCursor({cursor.x, cursor.y+1});
            }
            else if (checkPressed(up))
            {
                cursor = (cursor.y > 0) ? Point(cursor.x, cursor.y-1) : cursor ;
            }
            else
            {
                char str[50] = "";
                keypressName(str);
                
                if(checkPressed("Control"))
                {
                    if (str[0] == 'C' || str[0] == 'c')
                    {
                        copySelection();
                    }
                    else if(str[0] == 'V' || str[0] == 'v')
                    {
                        pasteSelection();
                    }
                    else if(str[0] == 'X' || str[0] == 'x')
                    {
                        copySelection();
                        clearSelection();
                    }
                }
                else if (str[0] != '\0') 
                {
                    copy_start = {-1,-1};
                    addCharAtCursor(str);
                }
                drawText();
            }
            trackCopy();
            checkScroll();
            highlightSelection(BLACK);
            drawCursor(BLACK);
            drawPosition();

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