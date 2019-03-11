#include"fifo.h"

#define NO_MILLS_DRAW 40

typedef struct coords_t {
    int x,
        y;
} coords;

typedef struct gameInfo_t {

    GtkWidget       *widget,
                    *darea;

    cairo_surface_t *surface;
    coords          *array;

    gchar           *nick1,
                    *nick2;

    gboolean         hints;

    gint             player,
                     board1,
                     board2,
                     disc,
                     silo,
                     drag,
                     phase,
                     mill;

    GdkRGBA          color1,
                     color2;
    
    gboolean         turn;

    guint            data,
                     nomills;
                  
} gameInfo;

typedef struct moveInfo_t {
     
    gint             nfield,
                     pfield;

} moveInfo;

/* main functions */

void initInfo(gameInfo *info);

void initMenu(GtkWidget *window, gameInfo *info);

void initGame(gameInfo *info);

gboolean painter(gameInfo *info);

/* dialogs */

void showVictory(gameInfo *info, int option);

void askForRestart(GtkWidget *widget, gameInfo *info);

void showQuestion(gameInfo *info, int option);

void showError(char *komunikat);

void showDraw(gameInfo *info);

void surrender(GtkWidget *widget, gameInfo *info);

void endGame(gameInfo *info);

/* mechanic helpers */

int checkPosition(GtkWidget *widget, gameInfo *info, int x, int y);

bool canPlayerMoveHere(gameInfo *info, int src, int dest);

int getAdjacentFields(gameInfo* info, int n);

int countLitBits(int n);

int millQuery(int board);

int freeQuery(gameInfo *info);

bool isInMill(int pawn, int board);

bool doAllFormMill(int board);

/* other helpers */

void paintSetup(GtkWidget *widget, gameInfo *info);

void paintStaticBoard(GtkWidget *widget,
                      gameInfo  *info,
                      cairo_t   *cr,
                      int radius, int width, int height);

void drawDynamic(GtkWidget *widget,
                 gameInfo  *info,
                 gint       x,
                 gint       y);


/* drawing callbacks */

gboolean configureCb(GtkWidget         *widget,
                     GdkEventConfigure *event,
                     gameInfo          *info);
                    
gboolean drawCb(GtkWidget *widget,
                cairo_t   *cr,
                gameInfo  *info);

gboolean motionNotifyCb(GtkWidget      *widget,
                        GdkEventMotion *event,
                        gameInfo       *info);

gboolean buttonPressCb(GtkWidget       *widget,
                        GdkEventButton *event,
                        gameInfo       *info);

gboolean buttonReleaseCb(GtkWidget      *widget,
                         GdkEventButton *event,
                         gameInfo       *info);

/* general helpers */

void showInfo(gameInfo *info);

/* move function */

void makeMove(int prev, int next, gameInfo *info);

/* message helpers (main.c) */

void getMessage(gchar *msg);

void sendMessage(gchar *msg);

/* move message helpers */

void sendMoveMessage(int prev, int next, gameInfo *info);

void readMoveMessage(gchar *msg, moveInfo *minfo);

gboolean readMessage(gameInfo *info);

/* general helpers (main.c) */

void destroyInfo(gameInfo *info);

/* hello helpers */

gboolean getHelloMessage(gameInfo *info);

void readStartInfo(GtkWidget *widget, gameInfo *info);

void sendHelloMessage(gameInfo *info); 

void readHelloMessage(gchar *msg, gameInfo *info); 

bool equalColors(GdkRGBA *color1, GdkRGBA *color2);
