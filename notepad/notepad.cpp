#include "graphics.h"
#include <string.h>
#include <shlobj.h>
#include <cstdio>
#include <string>

int i, j = 0, gd = DETECT, gm;

//Limited to between 0 and 99999 - Height/20
static int start_num = 0;
static int width_num = 0;
static int counter = 0;
static int cursor_col = BLACK;
static bool cursor_down = false;

int Width = 800;
int Height = 624;
const int xoffset = 62;
const int yoffset = 24;
const int charwidth = 12;
const int charheight = 20;

char file_name_buf[MAX_PATH] = "";

char blankstr[6] = {' ',' ',' ',' ',' ','\0'};
char rowstr[6] = {'R','o','w',':',' ','\0'};
char colstr[6] = {'C','o','l',':',' ','\0'};

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

void drawFillRect(Point a, Point b, int col);
void saveFile(char *fp);
void loadFile(char *fp);

class Button
{
    Point topLeft;
    std::string text;
    bool active = true;
    bool saveAction = false;
    public:
        Button(Point p,std::string msg,bool save)
        {
            topLeft = p;
            text = msg;
            saveAction = save;
            drawFillRect(topLeft,{topLeft.x+charwidth*((int)text.length()),topLeft.y+charheight},BLACK);
        }
        void drawButton()
        {
            this->active = (mousex() >= topLeft.x && mousex() <= topLeft.x+charwidth*((int)text.length()) &&
                mousey() >= topLeft.y && mousey() <= topLeft.y+charheight);
            int old_col = getbkcolor();
            drawFillRect({topLeft.x,topLeft.y},
                         {topLeft.x+charwidth*((int)text.length()),topLeft.y+charheight},
                         (this->active)?DARKGRAY:LIGHTGRAY);
            setbkcolor((this->active)?DARKGRAY:LIGHTGRAY);
            outtextxy(topLeft.x,topLeft.y,const_cast<char *>(text.c_str()));
            setbkcolor(old_col);
        }

        void buttonAction()
        {
            if (this->active && saveAction)
            {
                if (strlen(file_name_buf) > 0)
                {
                    saveFile(file_name_buf);
                }
                else 
                {
                    char path[MAX_PATH];
                    browseFolderPath(path);
                    saveFile(path);
                }
            }
            else if (this->active)
            {
                browseFilePath(file_name_buf);
                loadFile(file_name_buf);
            }
        }
};

static Point cursor = {0,0};
static Point copy_start = {-1,-1};

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

//Fills a rectangle with color at point a(top, left) and point b(bottom,left) 
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

//Converts the text position to a coordinate on the screen
Point pixelToCoordinate(Point a)
{
    return Point(xoffset+a.x*charwidth,yoffset+(a.y-start_num)*charheight);
}

//Converts the coordinate on the screen to the text position
Point coordinateToPixel(Point a)
{
    return Point((a.x-xoffset)/charwidth,(a.y-yoffset)/charheight);
}

//Draws the string at Point a on the screen
void drawText(Point a, char* text) 
{
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

//Returns the visible cursor instead of pointed position
int getFirstVisiblePosition()
{
    int pos = 0;
    int row = 0;
    int col = 0;
    while(buf[pos] != '\0' && row < start_num || (row == start_num && col < width_num))
    {
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

//Draws the cursor using the color
void drawCursor(int col)
{
    if (cursor.x < 0 || cursor.y < start_num) { return; }
    int old_col = getcolor();
    Point cur = getActualCursor(cursor);
    Point bgn = pixelToCoordinate(Point{cur.x-width_num,cur.y}), dst = pixelToCoordinate(Point(cur.x-width_num,cur.y+1));
    setcolor(col);
    line(bgn.x,bgn.y,dst.x,dst.y);
    setcolor(old_col);
}

//Adds the character at the cursor
void addCharAtCursor(char *str)
{
    //Gets the position to add it into
    int pos = getTextPosition(cursor);

    //Clear all written strings after this
    cursor = (str[0] == '\n') ? Point(0, cursor.y+1) : cursor = Point(cursor.x+1,cursor.y);

    //Adds the new char in and moves all other chars 
    for(int i = strlen(buf); i >= pos; i--)
    {
        buf[i+1] = buf[i];
    }
    buf[pos] = str[0];
}

//Removes the character at the cursor
void removeCharAtCursor()
{
    //Gets the position to add it into
    int pos = getTextPosition(cursor);

    //Clear all written strings after this
    cursor = (buf[pos-1] == '\n') ? getActualCursor({9999, cursor.y-1}) : cursor = {cursor.x-1,cursor.y};

    for(int i = pos; i <= strlen(buf); i++)
    {
        buf[i-1] = buf[i];
    }
}

//Draws all the text on the screen
void drawText()
{
    drawFillRect(Point(xoffset-1,yoffset),Point(Width, Height), WHITE);
    int row = 0, col = 0, pos = 0;
    while (buf[pos] != '\0') {
        if (buf[pos] == '\n')
        {
            row++;
            col = 0;
        }
        else if (row >= start_num && row < start_num + Height/20)
        {
            if (col >= width_num && col <= width_num + Width/12)
            {
                char str[2] = {buf[pos],'\0'};
                drawText(pixelToCoordinate({col-width_num,row}), str);
            }
            col++;
        }
        pos++;
    }
}

//Draws the position of the cursor
void drawPosition()
{
    Point cur = getActualCursor(cursor);
    char str[5];
    itoa(cur.y,str,10);
    drawText({50*12,0},rowstr);
    drawText({60*12,0},colstr);
    drawText({54*12,0},blankstr);
    drawText({54*12,0},str);
    itoa(cur.x,str,10);
    drawText({64*12,0},blankstr);
    drawText({64*12,0},str);
}

//Draws the sidebar numbers
void drawSidebar()
{
    for(int i = 0; i < (Height - (yoffset+2))/20; i++)
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

//Checks the scroll start position
void checkScroll()
{
    Point cur = getActualCursor(cursor);
    if (cur.y < start_num || cur.y >= start_num + ((Height - (yoffset+2))/20))
    {
        start_num += (cursor.y < start_num) ? -1 : 1;
    }
    if (cur.x < width_num || cur.x >= width_num +(Width - (xoffset+2))/12)
    {
        width_num += (cur.x < width_num) ? -1 : cur.x - (Width - (xoffset+2))/12;
    }
}

//Draws the copyable selection
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

//Tracks the copyable selection
void trackCopy()
{
    copy_start = (!checkPressed("Shift")) ? Point(-1,-1) : copy_start ;
}

//Loads a file into the notepad
void loadFile(char *path)
{
    FILE *fp = fopen(path,"r");
    if(fp)
    {
        int acc = 0;
        while((buf[acc] = getc(fp)) != EOF) { acc++; }
        buf[acc] = '\0';
        fclose(fp);
        cursor = {0,0};
    }
}

//Saves a file from the notepad to the filesystem
void saveFile(char *path)
{
    FILE *fp = fopen(path,"w");
    if(fp)
    {
        int acc = 0;
        while(buf[acc] != '\0') {
            putc(buf[acc],fp);
            acc++;
        }
        fclose(fp);
    }
}

//Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
}

//Copies the selection to the clipboard
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
    }
}

//Removes the selection
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

//Copies the clipboard to the screen
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
}

int main()
{
    GetDesktopResolution(Width,Height);
    initwindow(Width, Height, "Notepad",0,0,true);
    Button *fOpenButton = new Button(Point(10,0),"Open File",false);
    Button *fSaveButton = new Button(Point(150,0),"Save File",true);
    settextstyle(DEFAULT_FONT, 0, 0);
    setbkcolor(WHITE);

    while (true) {
        if (ismouseclick(WM_MOUSEWHEEL))
        {
            int scroll_num = getmousescroll();
            if (scroll_num != 0 && start_num - sgn(scroll_num) >= 0)
            {
                drawCursor(WHITE);
                start_num -= sgn(scroll_num);
                drawCursor(BLACK);
            }
        }
        
        if(ismouseclick(WM_LBUTTONUP))
        {
            fOpenButton->buttonAction();
            fSaveButton->buttonAction();
            drawCursor(WHITE);
            cursor = coordinateToPixel({mousex(),mousey()});
            drawCursor(BLACK);
            clearmouseclick(WM_LBUTTONUP);
            clearmouseclick(WM_LBUTTONDOWN);
            cursor_down = false;
        }
        else if(ismouseclick(WM_LBUTTONDOWN) && !cursor_down)
        {
            highlightSelection(WHITE);
            copy_start = coordinateToPixel({mousex(),mousey()});
            cursor_down = true;
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
                }
            }
            else if (checkPressed("Left")) 
            {
                cursor = (cursor.x > 0) 
                         ? Point(getActualCursor(cursor).x-1,cursor.y) 
                         : (cursor.x == 0 and cursor.y > 0)
                           ? getActualCursor({9999,cursor.y-1})
                           : cursor ;
            }
            else if (checkPressed("Right"))
            {
                char c = buf[getTextPosition(cursor)];
                cursor = (c == '\n')
                         ? Point(0, cursor.y+1)
                         : (c == '\0')
                           ? cursor
                           : Point(cursor.x+1,cursor.y);
            }
            else if (checkPressed("Down"))
            {
                cursor = getActualCursor({cursor.x, cursor.y+1});
            }
            else if (checkPressed("Up"))
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
                    else if(str[0] == 'O' || str[0] == 'o')
                    {
                        browseFilePath(file_name_buf);
                        loadFile(file_name_buf);
                    }
                    else if(str[0] == 'S' || str[0] == 's')
                    {
                        if (strlen(file_name_buf) > 0)
                        {
                            saveFile(file_name_buf);
                        }
                        else
                        {
                            char path[MAX_PATH];
                            browseFolderPath(path);
                            char fname[14] = "/new_file.txt";
                            strcat(path,fname);
                            saveFile(path);
                        }
                    }
                }
                else if (str[0] != '\0') 
                {
                    copy_start = {-1,-1};
                    addCharAtCursor(str);
                }
            }
            trackCopy();
            checkScroll();
        }
        drawFillRect({0,0},{Width,yoffset-1},DARKGRAY);
        drawFillRect({0,yoffset},{xoffset-2,Height},LIGHTGRAY);
        drawSidebar();
        drawText();
        highlightSelection(BLACK);
        drawCursor(cursor_col);
        drawPosition();
        if (counter % 50 == 0) {
            cursor_col = BLACK;
            drawCursor(cursor_col);
        }
        else if (counter % 25 == 0) {
            cursor_col = WHITE;
            drawCursor(cursor_col);
        }
        fOpenButton->drawButton();
        fSaveButton->drawButton();
        counter++;
        swapbuffers();
        delay(1);
    }
    getchar();
    return 0;
}