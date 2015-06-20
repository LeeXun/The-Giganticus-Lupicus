#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <algorithm>
#include <math.h>
#include <ctype.h>
#include <mmsystem.h>
#include <dsound.h>
#define GRID_WIDTH  79
#define GRID_HEIGHT 24
#define BLOCK_WIDTH 10
#define BLOCK_HEIGHT 23
#define BLOCK_TIMER 1000
#define BUFF_TIMER 200
#define MAX_LUPI_ATTACK_SIZE 20
#define MAX_PLAYER_ATTACK_SIZE 100
#define mSPF 50
#define LUPI_HP 500
#define PLAYER_HP 10
#define PLAYER_BUFF 10
#define LOST sound/Lost.wav
#define WIN sound/Won.wav
CHAR_INFO screenBuffer[GRID_WIDTH * GRID_HEIGHT];	// This is our buffer that will hold all the data we wish to display to the screen
SMALL_RECT drawRect = {0, 0, (GRID_WIDTH),(GRID_HEIGHT)};

COORD gridSize = {GRID_WIDTH , GRID_HEIGHT};		// This is a COORD that holds the width and height of our grid that we will display
COORD zeroZero = {0, 0};							// This tells us the upper left corner to write from
HANDLE OutputH= GetStdHandle(STD_OUTPUT_HANDLE);
double game_time = 0;
int music = 0;
char buffers[50];
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
enum {left = -1, right = 1, up = -1, down = 1};
struct Attack
{
    int x,y;
    int dirx = left;
    int diry = up*0;
    bool exist = false;
    int speed = 2;
};
struct Player
{
    char name = 'A';
    int x=GRID_WIDTH/2,y=GRID_HEIGHT-2;
    bool is_uping = false;
    bool is_falling=false;
    bool is_jumping = false;
    bool is_dead = false;
    int jump_height = 5;
    int jump_base = GRID_HEIGHT-2;
    int color = 15;
    int hp = PLAYER_HP;
    int buff = PLAYER_BUFF;
    int direction = right;
    Attack attack[MAX_PLAYER_ATTACK_SIZE];
};
struct Plist
{
    Player p[3];
    int size = 2;
};
Plist plist;

enum {nothing,breathing,attacking,stomping};
struct Lupi
{
    int x=60,y=13;
    Attack attack[MAX_LUPI_ATTACK_SIZE];
    int hard_size=1;
    int mov=1;
    int motion = nothing;
    int hp = LUPI_HP;
};
Lupi lupi;
Lupi Reset;
struct Buff
{
    int x,y;
    bool is_exist = false;
};
struct BuffList
{
    Buff buff[5];
    int buff_size = 5;
};
struct Nature
{
    int timer[5]= {BLOCK_TIMER,BUFF_TIMER,BLOCK_TIMER,BLOCK_TIMER,BLOCK_TIMER};
};
struct Block
{
    int x,y;
};
struct BlockList
{
    Block block[79*24];
    int block_size = 0;
};
BlockList blist;
BuffList bulist;
Nature nature;
bool NeedsBack(int ind)
{
    char c = screenBuffer[plist.p[ind].x+plist.p[ind].y*GRID_WIDTH].Char.AsciiChar;
    return c!=' '&&c!='@'&&c!='+';
}
bool is_Buff(int ind)
{
    char c = screenBuffer[plist.p[ind].x+plist.p[ind].y*GRID_WIDTH].Char.AsciiChar;
    return c=='+';
}
bool is_attacked(int ind)
{
    char c = screenBuffer[plist.p[ind].x+plist.p[ind].y*GRID_WIDTH].Char.AsciiChar;
    return c=='@';
}
void CPattack(int x,int y)
{
    if(screenBuffer[x + y * GRID_WIDTH].Char.AsciiChar == ' '&&(screenBuffer[x + y * GRID_WIDTH].Attributes == 15+15*16||screenBuffer[x + y * GRID_WIDTH].Attributes == 7+7*16))
    {
        lupi.hp -=1;
    }
    else if((screenBuffer[x + y * GRID_WIDTH].Char.AsciiChar == '/')&&(screenBuffer[x + y * GRID_WIDTH].Attributes == 15+15*16||screenBuffer[x + y * GRID_WIDTH].Attributes == 7+7*16))
    {
        lupi.hp -=1;
    }
    for(int i=x; i<x+1; i++)
    {
        screenBuffer[i + y * GRID_WIDTH].Char.AsciiChar = '*';
        screenBuffer[i + y * GRID_WIDTH].Attributes = 6;
    }
}
void Draw_Attack(int ind)
{
    Player *p = &plist.p[ind];
    for(int i=0; i<MAX_PLAYER_ATTACK_SIZE; i++)
    {
        if(p->attack[i].exist)
        {
            p->attack[i].x +=p->attack[i].dirx*(p->attack[i].speed);
            p->attack[i].y +=p->attack[i].diry;
            if(p->attack[i].x < 0||p->attack[i].x > 77)
            {
                p->attack[i].exist = false;
            }
            else if(p->attack[i].y < 1||p->attack[i].y > 22)
            {
                p->attack[i].exist = false;
            }
            else
                CPattack(p->attack[i].x,p->attack[i].y);
        }
    }
}
void change_position(int ind)
{
    Player *p = &plist.p[ind];
    bool upKey;
    bool leftKey;
    bool rightKey;
    bool attackKey;
    void jump(int ind);
    if(ind==0)
    {
        upKey = GetAsyncKeyState(VK_UP) !=0;
        leftKey = GetAsyncKeyState(VK_LEFT) !=0;
        rightKey = GetAsyncKeyState(VK_RIGHT) !=0;
        attackKey = GetAsyncKeyState(VK_DOWN) !=0;
    }
    else if(ind==1)
    {
        upKey = GetAsyncKeyState(0x57) !=0;
        leftKey = GetAsyncKeyState(0x41) !=0;
        rightKey = GetAsyncKeyState(0x44) !=0;
        attackKey = GetAsyncKeyState(0x53) !=0;
    }
    if(is_Buff(ind))
    {
        p->buff +=10;
        for(int i=0; i<5; i++)
        {
            if(bulist.buff[i].x == p->x)
            {
                bulist.buff[i].is_exist = false;
            }
        }
    }
    else if(NeedsBack(ind))//Find way to stand
    {
        int i=1;
        int x = p->x;
        for(i=1; i<4; i++)
        {
            p->x -= i;
            if(!NeedsBack(ind)&&p->x<80&&p->x>-1)
                break;
            p->x += i+1;
            if(!NeedsBack(ind)&&p->x<80&&p->x>-1)
                break;
        }
        if(i==4)
        {
            p->x = x;
        }
    }//Find way to stand
    if(p->is_jumping)
    {
        p->is_jumping = true;
        jump(ind);
    }
    else if(screenBuffer[p->x+(p->y+1)*GRID_WIDTH].Char.AsciiChar==' '||screenBuffer[p->x+(p->y+1)*GRID_WIDTH].Char.AsciiChar=='@')
    {
        p->is_jumping = true;
        p->is_falling = true;
        jump(ind);
    }
    if(upKey&&!p->is_jumping)
    {
        p->is_jumping = true;
        p->is_uping = true;
        jump(ind);
    }
    if(rightKey)
    {
        p->direction = right;
        p->x = std::min(GRID_WIDTH-1,p->x+1);
        if(NeedsBack(ind))
        {
            p->x-=1;
        }
    }
    if(leftKey)
    {
        p->direction = left;
        p->x = std::max(0,p->x-1);
        if(NeedsBack(ind))
        {
            p->x+=1;
        }
    }
    if(attackKey)
    {
        if(p->buff>0)
        {
            int ind = -1;
            for(int i=0; i<MAX_PLAYER_ATTACK_SIZE; i++)
            {
                if(!p->attack[i].exist)
                {
                    ind = i;
                    break;
                }
            }
            p->attack[ind].exist = true;
            p->buff-=1;
            p->attack[ind].x = p->x+p->direction+1;
            p->attack[ind].y = p->y;
            p->attack[ind].dirx = p->direction;
        }
    }
}
void jump(int ind)
{
    plist.p[ind].is_jumping = true;
    if(plist.p[ind].is_uping)
    {
        if(plist.p[ind].y!=plist.p[ind].jump_base-plist.p[ind].jump_height)
        {
            plist.p[ind].y--;
            if(NeedsBack(ind))
            {
                plist.p[ind].y++;
                plist.p[ind].is_falling = true;
                plist.p[ind].is_uping = false;
            }
        }
        else
        {
            plist.p[ind].y++;
            plist.p[ind].is_falling = true;
            plist.p[ind].is_uping = false;
        }
    }
    if(plist.p[ind].is_falling)
    {
        if(NeedsBack(ind)||plist.p[ind].y == GRID_HEIGHT-1)
        {
            plist.p[ind].y--;
            plist.p[ind].is_falling = false;
            plist.p[ind].is_jumping = false;
            plist.p[ind].jump_base = plist.p[ind].y;
        }
        plist.p[ind].y++;
        if(NeedsBack(ind)||plist.p[ind].y == GRID_HEIGHT-1)
        {
            plist.p[ind].y--;
            plist.p[ind].is_falling = false;
            plist.p[ind].is_jumping = false;
            plist.p[ind].jump_base = plist.p[ind].y;
        }
    }
}
void Cplayer(int ind)
{
    screenBuffer[plist.p[ind].x + plist.p[ind].y * GRID_WIDTH].Char.AsciiChar = plist.p[ind].name;
    screenBuffer[plist.p[ind].x + plist.p[ind].y * GRID_WIDTH].Attributes = plist.p[ind].color;
}
void InputPlayer()
{
    for(int i=0; i<plist.size; i++)
    {
        if(!plist.p[i].is_dead)
        {
            if(is_attacked(i))
            {
                plist.p[i].hp-=1;
                if(plist.p[i].hp<=0)
                {
                    plist.p[i].is_dead = true;
                    plist.p[i].color = 13;
                }
            }
            change_position(i);
        }
        Cplayer(i);
        Draw_Attack(i);
    }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void CNature()
{
    for(int i=0; i<GRID_WIDTH; i++)//Floor
    {
        screenBuffer[i + (GRID_HEIGHT-1) * GRID_WIDTH].Char.AsciiChar = 0x80;
        screenBuffer[i + (GRID_HEIGHT-1) * GRID_WIDTH].Attributes = 255;
    }//Floor
    nature.timer[0] +=1;//Generate Blocks
    if(nature.timer[0]>=BLOCK_TIMER)
    {
        blist.block_size = 0;
        for(int i=0; i<GRID_WIDTH; i++)
        {
            for(int j=0; j<BLOCK_HEIGHT-1; j++)
            {
                int r=rand()%50;
                if(r==0)
                {
                    blist.block_size+=1;
                    blist.block[blist.block_size].x = i;
                    blist.block[blist.block_size].y = j;
                    for(int k=0; k<BLOCK_WIDTH; k++)
                    {
                        screenBuffer[i+k + j * GRID_WIDTH].Char.AsciiChar = 0x80;
                        screenBuffer[i+k + j * GRID_WIDTH].Attributes = 8;
                    }
                }
            }
        }
        nature.timer[0]=0;
    }
    else
    {
        for(int i=0; i<blist.block_size; i++)
        {
            for(int k=0; k<BLOCK_WIDTH; k++)
            {
                screenBuffer[blist.block[i].x+k + blist.block[i].y * GRID_WIDTH].Char.AsciiChar = 0x80;
                screenBuffer[blist.block[i].x+k + blist.block[i].y * GRID_WIDTH].Attributes = 8;
            }
        }
    }//Generate Blocks
    nature.timer[1] +=1;//Generate Buffs
    if(nature.timer[1]>=BUFF_TIMER)
    {
        for(int i=0; i<5; i++)
        {
            bulist.buff[i].is_exist = true;
            bulist.buff[i].x = rand()%78;
            bulist.buff[i].y = rand()%23;
        }
        nature.timer[1]=0;
    }
    else
    {
        for(int i=0; i<5; i++)
        {
            if(bulist.buff[i].is_exist)
            {
                screenBuffer[bulist.buff[i].x + bulist.buff[i].y * GRID_WIDTH].Char.AsciiChar = '+';
                screenBuffer[bulist.buff[i].x + bulist.buff[i].y * GRID_WIDTH].Attributes = 12+12*16;
            }
        }
    }//Generate Buffs
}
void Nature()
{
    CNature();
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void CleanCanvas()
{
    for(int y = 0; y < gridSize.Y; y++)						// This fills in the columns (y value) of our buffer
    {
        for(int x = 0; x < gridSize.X; x++)				// This fills in the rows (x value) of our buffer.
        {
            screenBuffer[x + y * GRID_WIDTH].Char.AsciiChar = ' ';
            screenBuffer[x + y * GRID_WIDTH].Attributes = FOREGROUND_GREEN;
        }
    }
}
void CLupi(int x,int y)
{
    if(lupi.motion == stomping)
    {
        y--;
    }
    for(int i=x; i<x+4; i++) //Head
    {
        for(int j=y; j<y+2; j++)
        {
            screenBuffer[i + j * GRID_WIDTH].Char.AsciiChar = '/';
            screenBuffer[i + j * GRID_WIDTH].Attributes = 15+15*16;
        }
    }//Head
    for(int i=x-3; i<x+7; i++) //Shoulder
    {
        for(int j=y+2; j<y+3; j++)
        {
            screenBuffer[i + j * GRID_WIDTH].Char.AsciiChar = '/';
            screenBuffer[i + j * GRID_WIDTH].Attributes = 7+7*16;
        }
    }//Shoulder
    for(int i=x-1; i<x+5; i++) //body
    {
        for(int j=y+3; j<y+7; j++)
        {
            screenBuffer[i + j * GRID_WIDTH].Char.AsciiChar = '/';
            screenBuffer[i + j * GRID_WIDTH].Attributes = 7+7*16;
            if(j==y+4||j==y+5)
            {
                screenBuffer[(x-1) + j * GRID_WIDTH].Char.AsciiChar = '/';
                screenBuffer[(x-1) + j * GRID_WIDTH].Attributes = 15+15*16;
                screenBuffer[(x+4) + j * GRID_WIDTH].Char.AsciiChar = '/';
                screenBuffer[(x+4) + j * GRID_WIDTH].Attributes = 15+15*16;
            }
        }
    }

    //body
    //down Hands
    int j;
    if(lupi.motion==breathing)
    {
        j=y+1;
    }
    else
    {
        j=y+2;
    }
    for(int i=0; j<y+8; j++,i++)
    {
        screenBuffer[(x-3-i) + j * GRID_WIDTH].Char.AsciiChar = '/';
        screenBuffer[(x-3-i) + j * GRID_WIDTH].Attributes = 7+7*16;
        screenBuffer[(x+6+i) + j * GRID_WIDTH].Char.AsciiChar = '/';
        screenBuffer[(x+6+i) + j * GRID_WIDTH].Attributes = 7+7*16;
        if(j>=y+3||j<=j+6)
        {
            screenBuffer[(x-3-i)-1 + j * GRID_WIDTH].Char.AsciiChar = '/';
            screenBuffer[(x-3-i)-1 + j * GRID_WIDTH].Attributes = 15+15*16;
            screenBuffer[(x+6+i)+1 + j * GRID_WIDTH].Char.AsciiChar = '/';
            screenBuffer[(x+6+i)+1 + j * GRID_WIDTH].Attributes = 15+15*16;
        }
    }//down Hands
    /*else{
    //up Hands
            for(int j=y+2,i=0;j>y-5;j--,i++){
    screenBuffer[(x-3-i) + j * GRID_WIDTH].Char.AsciiChar = 0x80;
    screenBuffer[(x-3-i) + j * GRID_WIDTH].Attributes = 7+7*16;
    screenBuffer[(x+6+i) + j * GRID_WIDTH].Char.AsciiChar = 0x80;
    screenBuffer[(x+6+i) + j * GRID_WIDTH].Attributes = 7+7*16;
    }//up Hands
    }*/
    for(int i=x-2; i<x+6; i++) //But
    {
        screenBuffer[i + (y+6) * GRID_WIDTH].Char.AsciiChar = '/';
        screenBuffer[i + (y+6) * GRID_WIDTH].Attributes = 7+7*16;
    }//But
    for(int j=y+6,i=0; j<y+10; j++,i++) //Feet
    {
        screenBuffer[(x-1-i) + j * GRID_WIDTH].Char.AsciiChar = ' ';
        screenBuffer[(x-1-i) + j * GRID_WIDTH].Attributes = 7+7*16;
        screenBuffer[(x+4+i) + j * GRID_WIDTH].Char.AsciiChar = ' ';
        screenBuffer[(x+4+i) + j * GRID_WIDTH].Attributes = 7+7*16;
        if(j>=y+7||j<=j+9)
        {
            screenBuffer[(x-1-i)-1 + j * GRID_WIDTH].Char.AsciiChar = ' ';
            screenBuffer[(x-1-i)-1 + j * GRID_WIDTH].Attributes = 15+15*16;
            screenBuffer[(x+4+i)+1 + j * GRID_WIDTH].Char.AsciiChar = ' ';
            screenBuffer[(x+4+i)+1 + j * GRID_WIDTH].Attributes = 15+15*16;
        }
    }//Feet
}
void Lupi()
{
    void Cattack(int x,int y);
    int percentage = 100;
    int r = rand()%percentage;
    if(r<5&&lupi.x>10&&lupi.x<79-13)///move
    {
        Player p;
        p.is_dead = true;
        for(int i=0; i<plist.size; i++)
        {
            if(!plist.p[i].is_dead)
            {
                p = plist.p[i];
            }
        }
        if(p.x<lupi.x)
        {
            lupi.mov=-1;
        }
        else if(p.x>lupi.x)
        {
            lupi.mov=1;
        }
        else
        {
            lupi.mov=0;
        }
        lupi.x+=lupi.mov;
    }///move
    switch(lupi.motion)
    {
    case nothing:
    {
        if(r<50)
        {
            lupi.motion = nothing;
        }
        if(r>=40&&r<50)
        {
            lupi.motion = attacking;
        }
        else
        {
            lupi.motion = breathing;
        }
        break;
    }
    case breathing:
    {
        if(r>=50)
        {
            lupi.motion = breathing;
        }
        if(r>=40&&r<50)
        {
            lupi.motion = attacking;
        }
        else
        {
            lupi.motion = nothing;
        }
        break;
    }
    case attacking:
    {

        break;
    }
    }
    CLupi(lupi.x,lupi.y);
    for(int i=0; i<MAX_LUPI_ATTACK_SIZE; i++)
    {
        if(!lupi.attack[i].exist)
        {
            lupi.attack[i].x = lupi.x+2;
            lupi.attack[i].y = lupi.y+2;
            int diry = rand()%3;
            int density = rand()%5+3;
            int dirx = density-diry;
            if(rand()%2==0)
                diry *= -1;
            if(rand()%2==0)
                dirx *= -1;
            if(dirx==0)
            {
                diry = 2;
                if(rand()%2==0)
                    diry *= -1;
            }
            if(diry==0)
            {
                dirx = 2;
                if(rand()%2==0)
                    dirx *= -1;
            }
            lupi.attack[i].exist = true;
            lupi.attack[i].diry = diry;
            lupi.attack[i].dirx = dirx;
        }
    }
    for(int i=0; i<MAX_LUPI_ATTACK_SIZE; i++)
    {
        if(lupi.attack[i].x <= 0||lupi.attack[i].x >= 79)
        {
            lupi.attack[i].exist = false;
        }
        else if(lupi.attack[i].y <= 1||lupi.attack[i].y >= 23)
        {
            lupi.attack[i].exist = false;
        }
        if(lupi.attack[i].exist)
        {
            lupi.attack[i].x +=lupi.attack[i].dirx;
            lupi.attack[i].y +=lupi.attack[i].diry;
            Cattack(lupi.attack[i].x,lupi.attack[i].y);
        }
    }
}///others attacks
void Cattack(int x,int y)
{
    if(x>0&&x<79&&y>0&&y<23)
    {
        screenBuffer[x + y * GRID_WIDTH].Char.AsciiChar = '@';
        screenBuffer[x + y * GRID_WIDTH].Attributes = 15;
    }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void gotoxy(int xpos, int ypos)
{
    COORD scrn;
    HANDLE hOuput = GetStdHandle(STD_OUTPUT_HANDLE);
    scrn.X = xpos;
    scrn.Y = ypos;
    SetConsoleCursorPosition(hOuput,scrn);
}
void Pause()
{
    void Open();
    bool pause = GetAsyncKeyState(VK_F2) !=0;
    if(pause)
    {
        while(true)
        {
            bool endpause = GetAsyncKeyState(VK_F1) !=0;
            if(endpause)
                break;
        }
    }
    bool esc = GetAsyncKeyState(VK_F4) !=0;
    if(esc)
    {
        Open();
    }
    game_time += mSPF;
}
HANDLE  hConsole= GetStdHandle(STD_OUTPUT_HANDLE);
void UI()
{
    SetConsoleTextAttribute(hConsole, 15);
    gotoxy(0,24);
    printf("A:(hp:%2d buff:%4d) B:(hp:%2d buff:%4d)",plist.p[0].hp,plist.p[0].buff,plist.p[1].hp,plist.p[1].buff);
    printf(" Lupi: hp:%4d ",lupi.hp);
    Pause();
    printf("Time: %4d.%1d sec", (int)game_time/1000, (int)game_time%1000/100);
    SetConsoleTextAttribute(hConsole, 8);
}

void LeaderBoards()
{
    system("cls");
    FILE *infp;
    infp = fopen("leaderboards.txt","r");
    if(!infp)
    {
        gotoxy(20,2);
        SetConsoleTextAttribute(hConsole, 15);
        printf("leaderboards.txt is not found...");
        SetConsoleTextAttribute(hConsole, 8);
        char c = getch();
        system("cls");
        return;
    }
    else
    {
        char teamname[20];
        gotoxy(33,2);
        printf("Team     Time(sec)");
        int line=3;
        int ranks = 1;
        SetConsoleTextAttribute(hConsole, 15);
        while(fscanf(infp,"%s",teamname)==1)
        {
            double gtime;
            fscanf(infp,"%lf",&gtime);
            gotoxy(26,line);
            printf("rank%2d %-10s %5g",ranks,teamname, gtime/1000);
            line +=1;
            ranks+=1;
        }
        SetConsoleTextAttribute(hConsole, 8);
        fclose(infp);
        char c = getch();
        system("cls");
    }
}
void Open()
{
    system("cls");
    void SetGame();
    PlaySound(TEXT("sound/Overture.wav"), NULL, SND_ASYNC|SND_LOOP);
    int cusor = 0;
    while(true)
    {
        gotoxy(33,5);
        if(cusor==0)
            SetConsoleTextAttribute(hConsole, 15);
        printf("Start Game");
        SetConsoleTextAttribute(hConsole, 8);
        gotoxy(33,8);
        if(cusor==1)
            SetConsoleTextAttribute(hConsole, 15);
        printf("Leaderboards");
        SetConsoleTextAttribute(hConsole, 8);
        gotoxy(33,11);
        if(cusor==2)
            SetConsoleTextAttribute(hConsole, 15);
        printf("Reset Game");
        SetConsoleTextAttribute(hConsole, 8);
        gotoxy(33,14);
        if(cusor==3)
            SetConsoleTextAttribute(hConsole, 15);
        printf("Exit");
        SetConsoleTextAttribute(hConsole, 8);
        switch(getch())
        {
        case 72:
        {
            if(cusor>0)
                cusor -=1;
            else
                cusor = 3;
            break;
        }    // key up

        case 80:    // key down
        {
            if(cusor<3)
                cusor +=1;
            else
                cusor = 0;
            break;
        }
        case '\r':
        {
            switch(cusor)
            {
            case 0:
            {
                PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
                PlaySound(TEXT("sound/Game1.wav"), NULL, SND_ASYNC|SND_LOOP);
                return;
            }
            case 1:
            {
                LeaderBoards();
                break;
            }
            case 2:
            {
                SetGame();
                PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
                PlaySound(TEXT("sound/Game1.wav"), NULL, SND_ASYNC|SND_LOOP);
                return;
            }
            case 3:
            {
                exit(0);
                break;
            }
            }
        }
        }
    }
}
struct Fplayer
{
    char teamname[20] = " ";
    double time = 0;
};
void Win()
{
    void SetGame();
    PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
    PlaySound("sound/Won.wav", NULL, SND_ASYNC|SND_LOOP);
    void SetGame();
    Fplayer fp[11];
    system("cls");
    gotoxy(33,5);
    SetConsoleTextAttribute(hConsole, 15);
    printf("You Win.....");
    Sleep(1000);
    system("cls");
    gotoxy(33,5);
    printf("Your time: %g",game_time/1000);
    Sleep(1000);
    system("cls");
    gotoxy(33,5);
    printf("Checking you Rank...");
    Sleep(1000);
    system("pause");
    FILE *outfp;
    FILE *infp;
    outfp = fopen("leaderboards.txt","a");
    fclose(outfp);
    infp = fopen("leaderboards.txt","r");
    system("cls");
    int line=3;
    int ranks = 1;
    int fsize =0;
    int newrank = -1;
    SetConsoleTextAttribute(hConsole, 15);
    while(fscanf(infp,"%s",fp[fsize].teamname)==1&&fsize<10)
    {
        fscanf(infp,"%lf",&fp[fsize].time);
        fsize+=1;
    }
    fclose(infp);
    /////////////////
    bool found = false;
    for(int i=0; i<fsize; i++)
    {
        if(fp[i].time>=game_time)
        {
            for(int j=fsize-1; j>i; j--)
            {
                fp[j].time = fp[j-1].time;
                strcpy(fp[j].teamname,fp[j-1].teamname);
            }
            char newname[20];
            gotoxy(25,5);
            printf("Congrats! Pls input your name!...>");
            scanf("%s",newname);
            strcpy(fp[i].teamname,newname);
            fp[i].time = game_time;
            found = true;
            fsize+=1;
            break;
        }
    }
    if(!found&&fsize<10)
    {
        char newname[20];
        gotoxy(25,5);
        printf("Yeah! Pls input your name!...>");
        scanf("%s",newname);
        strcpy(fp[fsize].teamname,newname);
        fp[fsize].time = game_time;
        found = true;
        fsize+=1;
    }

    /////////////////
    system("cls");
    gotoxy(33,2);
    SetConsoleTextAttribute(hConsole, 8);
    printf("Team     Time(sec)");
    SetConsoleTextAttribute(hConsole, 15);
    outfp = fopen("leaderboards.txt","w");
    for(int i=0; i<fsize&&(i<10); i++)
    {
        gotoxy(25,line);
        printf("rank%2d %-10s %5g",ranks,fp[i].teamname, fp[i].time/1000);
        fprintf(outfp,"%s\n%f\n",fp[i].teamname,fp[i].time);
        line +=1;
        ranks+=1;
    }
    SetConsoleTextAttribute(hConsole, 8);
    fclose(outfp);
    system("pause");
    SetGame();
    Open();
}
void Lose()
{
    system("cls");
    void SetGame();
    PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
    PlaySound("sound/Lost.wav", NULL, SND_ASYNC|SND_LOOP);
    gotoxy(33,5);
    SetConsoleTextAttribute(hConsole, 15);
    printf("You Lose.....");
    SetConsoleTextAttribute(hConsole, 8);
    Sleep(3000);
    gotoxy(33,5);
    SetConsoleTextAttribute(hConsole, 15);
    printf("Resetting Game...");
    SetConsoleTextAttribute(hConsole, 8);
    Sleep(2000);
    system("pause");
    system("cls");
    SetGame();
    Open();
}

void SetGame()
{
    for(int i=0;i<2;i++)
    {
        plist.p[i] = plist.p[2];
        if(i==0)
        {
            plist.p[i].name = 'A';
        }
        if(i==1)
        {
            plist.p[i].name = 'B';
        }
    }
    lupi = Reset;
    game_time = 0;
    strcpy(buffers," ");
}
void Music()
{
    bool M = GetAsyncKeyState(VK_F3) !=0;
    if(M)
    {
            if(music<4)
                music +=1;
            else
                music = 0;
        switch(music)
    {
    case 0:
        {
            PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
            PlaySound("sound/Game1.wav", NULL, SND_ASYNC|SND_LOOP);
            break;
        }
        case 1:
        {
            PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
            PlaySound("sound/Game2.wav", NULL, SND_ASYNC|SND_LOOP);
            break;
        }
        case 2:
        {
            PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
            PlaySound("sound/Game3.wav", NULL, SND_ASYNC|SND_LOOP);
            break;
        }
        case 3:
        {
            PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
            PlaySound("sound/Game4.wav", NULL, SND_ASYNC|SND_LOOP);
            break;
        }

    }
    }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
int main()
{
    srand(time(0));
    CleanCanvas();
    Open();
    SetGame();
    while(true)
    {
        Nature();
        Lupi();
        InputPlayer();
        WriteConsoleOutput(OutputH, screenBuffer, gridSize, zeroZero, &drawRect);
        UI();
        Music();
        Sleep(mSPF);
        CleanCanvas();
        if(lupi.hp<=0)
        {
            PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
            Win();
        }
        if(plist.p[0].is_dead&&plist.p[1].is_dead)
        {
            PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
            Lose();
        }
    }
    PlaySound(NULL, NULL, SND_ASYNC|SND_LOOP);
    return 0;
}
