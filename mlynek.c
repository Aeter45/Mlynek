#include"mlynek.h"

gboolean drawCb(GtkWidget *widget __attribute__((unused)),
                cairo_t   *cr,
                gameInfo  *info)
{
    cairo_set_source_surface(cr, info->surface, 0, 0);
    cairo_paint(cr);
    return FALSE;
}

gboolean configureCb(GtkWidget         *widget, 
                     GdkEventConfigure *event __attribute__((unused)), 
                     gameInfo          *info)
{
    paintSetup(widget, info);
    return FALSE;
}

gboolean motionNotifyCb(GtkWidget      *widget,
                        GdkEventMotion *event,
                        gameInfo       *info) 
{
    if(info->surface == NULL) return TRUE;

    if(info->drag != -1 && (event->state & GDK_BUTTON1_MASK))
         drawDynamic(widget, info, event->x, event->y);

    return TRUE;
}

gboolean buttonPressCb(GtkWidget      *widget,
                       GdkEventButton *event,
                       gameInfo       *info)
{
    gtk_widget_queue_draw(info->widget);

    if(info->turn == FALSE) return TRUE;
    
    gint click = checkPosition(widget, info, event->x, event->y); 

    if(click != -1) {

        if((((1<<click) & info->board2) > 0) && millQuery(info->board1)>0) {
            
            if(info->mill==0) return TRUE;
            if(isInMill(click,info->board2)&&(!doAllFormMill(info->board2))) return TRUE;

            info->board2 &= ~(1<<click);

            sendMoveMessage(click, -1, info);
            return TRUE;
        }
        
        if(info->mill==1) return TRUE;

        if(info->phase == 1) { 
            if(((1<<click) & info->board1)==0 && 
               ((1<<click) & info->board2)==0 && info->silo>0) {
                
                info->board1 |= 1<<click;
                info->silo--;
                sendMoveMessage(-1, click, info);
                return TRUE;
            }

            if(info->silo==0) info->phase=2;
        }
    }

    if(info->phase==1 || info->mill==1) return TRUE;

    if(info->drag!=(-1)||event->button!=GDK_BUTTON_PRIMARY) return TRUE;

    info->drag = click;
    if(click == -1) return TRUE;                             
    if((info->board1 & (1<<info->drag)) == 0) { 
        info->drag = -1;
        return TRUE;
    } 

    if(countLitBits(info->board1)<=3) info->phase=3;

    info->board1 &= ~(1<<info->drag);
    info->disc = 1;

    paintSetup(widget, info);
    drawDynamic(widget, info, event->x, event->y);
    gtk_widget_queue_draw(widget);
    return TRUE;
}

gboolean buttonReleaseCb(GtkWidget      *widget,
                         GdkEventButton *event,
                         gameInfo       *info) 
{    
    
    if(event->button!=GDK_BUTTON_PRIMARY) return TRUE;

    int dest = checkPosition(widget, info, event->x, event->y);
    if(info->drag != -1) {

        if(dest != -1 && canPlayerMoveHere(info, info->drag, dest)) { 
             info->board1 |= 1<<dest;
             sendMoveMessage(info->drag, dest, info);
        } 
        
        else if(dest != -1 && info->phase  == 3 && 
                (info->board1 & (1<<dest)) == 0 &&
                (info->board2 & (1<<dest)) == 0 &&
                info->drag != dest) 
        {
            info->board1 |= 1<<dest;
            sendMoveMessage(info->drag, dest, info);
        }
        else info->board1 |= 1<<info->drag;
        info->disc = 0;
    }
    info->drag = -1;

    paintSetup(widget,info);
    gtk_widget_queue_draw(widget);
    return TRUE;
}


void drawDynamic(GtkWidget *widget,
                 gameInfo  *info,
                 gint       x,
                 gint       y)
{
    guint radius = gtk_widget_get_allocated_height(widget)/26;
    paintSetup(widget, info);

    cairo_t *cr = cairo_create(info->surface);
    if(info->turn == TRUE) gdk_cairo_set_source_rgba(cr, &info->color1);
    else                   gdk_cairo_set_source_rgba(cr, &info->color2);

    cairo_arc(cr,
              x, y,
              radius,
              0, 2*G_PI);
    cairo_fill(cr);
    gtk_widget_queue_draw(widget);
    cairo_destroy(cr);
}

void paintSetup(GtkWidget *widget, gameInfo *info) 
{    
    
    guint width  = gtk_widget_get_allocated_width(widget),
          height = gtk_widget_get_allocated_height(widget),
          radius = height/26; 

    if(info->surface) 
        cairo_surface_destroy(info->surface);

    info->surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget), 
                                                       CAIRO_CONTENT_COLOR_ALPHA,
                                                       width, height);
    
    cairo_t *cr = cairo_create(info->surface);
    paintStaticBoard(widget, info, cr, radius, width, height);
    cairo_destroy(cr);
}

void paintStaticBoard(GtkWidget *widget,
                      gameInfo  *info, 
                      cairo_t   *cr, 
                      int radius, int width, int height) 
{

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    GdkRGBA color;
    
    gtk_render_background(context, cr, 0, 0, width, height);

    gtk_style_context_get_color(context,
                                gtk_style_context_get_state(context),
                                &color);

    gdk_cairo_set_source_rgba(cr, &color);

    cairo_set_line_width(cr, radius/2);
    cairo_rectangle(cr, radius, radius, (26-2)*radius, height-2*radius);
    cairo_stroke(cr);
    cairo_rectangle(cr, radius*5, radius*5, (26-10)*radius, height-10*radius);
    cairo_stroke(cr);
    cairo_rectangle(cr, radius*9, radius*9, (26-18)*radius, height-18*radius);
    cairo_stroke(cr);

    cairo_move_to(cr, 13*radius,    radius);
    cairo_line_to(cr, 13*radius,  9*radius);
    cairo_stroke(cr);
    cairo_move_to(cr,    radius, 13*radius);
    cairo_line_to(cr,  9*radius, 13*radius);
    cairo_stroke(cr);
    cairo_move_to(cr, 13*radius, 25*radius);
    cairo_line_to(cr, 13*radius, 17*radius);
    cairo_stroke(cr);    
    cairo_move_to(cr, 25*radius, 13*radius);
    cairo_line_to(cr, 17*radius, 13*radius);
    cairo_stroke(cr);

    int not1 = ~info->board1,
        not2 = ~info->board2;

    int adj;
    if(info->drag!= -1) 
         adj = getAdjacentFields(info, info->drag);
    else adj = 0;

    if(info->drag != -1 && info->phase == 3) adj = (not1 & not2);

    for(int i=0;i<24;++i) {
        gdk_cairo_set_source_rgba(cr,&color);

        if((not1 & (1<<i)) == 0) 
            gdk_cairo_set_source_rgba(cr, &info->color1);
        else if((not2 & (1<<i)) == 0)
            gdk_cairo_set_source_rgba(cr, &info->color2);

        cairo_arc(cr,
                  info->array[i].x*radius, info->array[i].y*radius,
                  radius,
                  0, 2*G_PI);

        cairo_fill(cr);

      /* drawing hints */        

        if( info->hints == TRUE && info->mill == 1 && 
            (((1<<i) & info->board2)>0) && 
            (!isInMill(i,info->board2)  || doAllFormMill(info->board2)) )
        {
            gdk_cairo_set_source_rgba(cr, &info->color1);
            cairo_set_line_width(cr,radius/10);
            cairo_arc(cr,
                      info->array[i].x*radius, info->array[i].y*radius,
                      radius/3,
                      0, 2*G_PI);
            cairo_fill(cr); 
        }

        if((i != info->drag) && (info->hints == TRUE) && (adj&(1<<i)) && (info->phase != 1)) {
            gdk_cairo_set_source_rgba(cr, &info->color1);
            cairo_set_line_width(cr,radius/10);
            cairo_arc(cr,
                      info->array[i].x*radius, info->array[i].y*radius,
                      radius/3,
                      0, 2*G_PI);
            cairo_fill(cr); 
        }

    }

    showInfo(info);

    if(info->disc == 1) return; 

    if(info->turn == TRUE               && 
       (countLitBits(info->board1) <= 2 ||(
       info->phase     == 2             && 
       freeQuery(info) == 0             && 
       info->mill      == 0 )))  //millQuery(info->board1) == 0 ))) 
    { 
        if(info->phase!=1) { 
            sendMoveMessage(-1, -1, info); 
            //endGame(info); 
            return;
        }
    }

    if(info->phase == 2 && countLitBits(info->board1)<=3) info->phase=3;

    if(info->phase != 1) return;
    int cnt = info->silo; 
    gdk_cairo_set_source_rgba(cr,&info->color1);
    while(cnt--) {
        cairo_arc(cr,
                  30*radius, (25-3*cnt)*radius,
                  radius,
                  0, 2*G_PI);
        cairo_fill(cr);
    } 
}

int countLitBits(int n) {
    int cnt=0;
    while(n) { n&=(n-1); cnt++; }
    return cnt;
} 

int millQuery(int board) 
{
    int mill1 = 7,      // 0b00000111
        mill2 = 193,    // 0b11000001
        mill3 = 131586, // 0b000000100000001000000010
        not   = ~board,
        cnt   = 0;
    for(int i=0;i<3;++i) {
        for(int j=0;j<3;++j) {
            if((mill1&not)==0) cnt++;
            mill1<<=2;
        }
        if((mill2&not)==0) cnt++;
        if((mill3&not)==0) cnt++;
        mill1<<=2;
        mill2<<=8;
        mill3<<=2;
    } 
    if((mill3&not)==0) cnt++;
    return cnt;
}

bool isInMill(int pawn, int board)
{
    int mill1=7,mill2=193,mill3=131586, 
		not   = ~board,
        cnt   = 0;
    for(int i=0;i<3;++i) {
        for(int j=0;j<3;++j) {
            if((mill1&not)==0&&(((1<<pawn)&mill1)>0)) cnt++;
            mill1<<=2;
        }
        if((mill2&not)==0&&(((1<<pawn)&mill2)>0)) cnt++;
        if((mill3&not)==0&&(((1<<pawn)&mill3)>0)) cnt++;
        mill1<<=2;
        mill2<<=8;
        mill3<<=2;
    } 
    if((mill3&not)==0&&(((1<<pawn)&mill2)>0)) cnt++;
    return (cnt>0);
} 

bool doAllFormMill(int board)
{
    for(int i=0;i<24;++i) if((((1<<i) & board)>0)&&!isInMill(i,board)) return false;
    return true;
}

bool canPlayerMoveHere(gameInfo *info, int src, int dest) 
{
    return (getAdjacentFields(info, src) & (1<<dest))!=0;
}

int getAdjacentFields(gameInfo *info, int n) 
{
    int adj = 0;
    adj|= ((n%8)?    (1<<(n-1)) : (1<<(n+7)) ) + 
          ((n%8<7)?  (1<<(n+1)) : (1<<(n-7)) );
    if(n%2) adj|= ( ((n/8)?   1<<(n-8) : 0) +
                    ((n/8<2)? 1<<(n+8) : 0) );
    adj&= ~(info->board1|info->board2);
    return adj;
}

int freeQuery(gameInfo *info) 
{
    int vis = 0, j = 1;
    for(int i=0;i<24;++i) {
        if(info->board1 & j) 
            vis|= getAdjacentFields(info, i);
        j<<=1;
    }
    return countLitBits(vis);
}

int checkPosition(GtkWidget *widget, gameInfo *info, int x, int y) 
{
    int r = gtk_widget_get_allocated_height(widget)/26,
           x2, y2, r2=r*r;
    for(int i=0;i<24;++i) {
        x2 = x-info->array[i].x*r; x2 *= x2;
        y2 = y-info->array[i].y*r; y2 *= y2;
        if(x2 + y2 <= r2) 
            return i;
    } 
    return -1;
}


/************************  general helpers  ************************/

void showInfo(gameInfo *info)
{
    gchar text[300];

    if(info->data == 1) {
        sprintf(text, "%s odrzucił prośbę o restart.", info->nick2);
    }    
    else if(info->turn == FALSE) 
        sprintf(text, "Poczekaj aż %s wykona ruch.", info->nick2);
    else if(info->mill == 1)//if(millQuery(info->board1)>0) /*****/
        sprintf(text, "Młynek! Możesz ściągnąć pion przeciwnika.");
    else if(info->phase < 2) 
        sprintf(text, "Wybierz pole na jakim chcesz postawić pion.");
    else 
        sprintf(text, "Aby ruszyć pionem, przeciągnij go w dostępne miejsce.");

    gtk_label_set_text(GTK_LABEL(info->widget), text);
    gtk_widget_queue_draw(info->widget);
    info->data = 0;
    //return (info->turn == TRUE)? FALSE : TRUE;
}

/*************************  move function  *************************/

void makeMove(int prev, int next, gameInfo *info) 
{
   
    int cnt = millQuery(info->board2);
    bool isin = false,
         isim = false;  

    if(prev == -1) {
        info->board2 |= (1<<next);
    }    
    else if(next == -1) {
        info->board1 &= ~(1<<prev);
    }
    else {
        isin = isInMill(prev, info->board2);
        info->board2 &= ~(1<<prev);
        info->board2 |= (1<<next);
        isim = isInMill(next, info->board2);
    }

    if(millQuery(info->board2) > cnt ||(isin && isim)) {
        gchar ups[5]; sprintf(ups, "%du", info->player);
        info->turn = FALSE;
        sendMessage(ups);
    }
    painter(info);
}

/************************* message helpers *************************/

void sendMoveMessage(int prev, int next, gameInfo *info) 
{
    info->turn = FALSE;

    if(info->mill == 1) info->nomills=0;
    else info->nomills++;

    info->mill = 0;

    gchar msg[50];

    if(info->nomills > NO_MILLS_DRAW/2) sprintf(msg,"%dd", info->player);
    else if(prev == -1 && next == -1) sprintf(msg,"%dme",info->player);
    else if(prev == -1) sprintf(msg, "%dmn%dk", info->player, next);
    else if(next == -1) sprintf(msg, "%dmd%dk", info->player, prev);
    else sprintf(msg, "%dmp%dn%dk", info->player, prev, next);

    sendMessage(msg);
	if(info->nomills > NO_MILLS_DRAW/2) showDraw(info);
}

void readMoveMessage(gchar *msg, moveInfo *minfo) 
{
    if(msg[1] != 'm') return;
    if(msg[2] == 'e') {
        minfo->nfield = -1;
        minfo->pfield = -1;
        return;
    }
    gchar *delim,
          *token;

    if(msg[2] == 'd' || msg[2] == 'n') {
        delim = "k";
        token = strtok(msg+3, delim);
        minfo->nfield = (msg[2] == 'd')? -1 : atoi(token);
        minfo->pfield = (msg[2] == 'n')? -1 : atoi(token);
        return;
    }
    if(msg[2] == 'p') {
        delim = "nk";
        token = strtok(msg+3, delim);
        minfo->pfield = atoi(token);
        token = strtok(NULL, delim);
        minfo->nfield = atoi(token);
        return;
    }     
}

gboolean readMessage(gameInfo *info) 
{
    gchar msg[50]; msg[0]='\0';
    getMessage(msg);
    //painter(info);

    if(msg[0] == '0' + info->player) return TRUE;

    switch(msg[1]) {

        case 'm':
            if(info->turn == TRUE) return TRUE;
            info->turn = TRUE;

            moveInfo minfo;
            readMoveMessage(msg, &minfo);
            if(minfo.nfield == -1 && minfo.pfield == -1) {
                showVictory(info, 2);
                //return FALSE;
            }
            makeMove(minfo.pfield, minfo.nfield, info);
            painter(info);
            break;

        case 'a':
            showQuestion(info,1);
            break;

        case 'y':
            initInfo(info);
            painter(info);
            break;

        case 'n':
            info->data = 1;
            painter(info);
            break;

        case 's':
            showVictory(info,1);
            return FALSE;

        case 'r':
            showQuestion(info,2);
            break;

        case 'e':
            endGame(info);
            break;

        case 'u':
            info->turn = TRUE;
            info->mill = 1; info->nomills = 0;
            painter(info);
            break;

        case 'd':
            showDraw(info);
            break;
        
        default: break;

    }
    //painter(info);
    return TRUE;
}

/************************** hello helpers **************************/

gboolean getHelloMessage(gameInfo *info)
{   
    gchar msg[50]; 
    getMessage(msg);
    
    if((msg[0] == '0' + info->player) || msg[1] != 'h')
        return TRUE;    

    readHelloMessage(msg, info);
    
    GtkWidget *box = gtk_bin_get_child(GTK_BIN(info->widget));
    GList *list = gtk_container_get_children(GTK_CONTAINER(box));
    list = list->next;
    GtkWidget *grid = list->data;
    
    GtkWidget *startButton = gtk_grid_get_child_at(GTK_GRID(grid),0, (info->player == 2)? 5 : 3);
    if(info->player == 2) gtk_widget_set_sensitive(startButton, TRUE);
    
    /* player A starts game */

    if(info->player == 1) initGame(info);
    
    return FALSE;
}

void destroyWaiting(GtkWidget *widget, gameInfo *info) { 
    if(widget != NULL) gtk_widget_destroy(widget); 
    info->darea = NULL; 
} 

void readStartInfo(GtkWidget *widget __attribute__((unused)),
                   gameInfo  *info)
{
    GtkWidget *box = gtk_bin_get_child(GTK_BIN(info->widget));
    GList *list = gtk_container_get_children(GTK_CONTAINER(box));
    list = list->next;
    GtkWidget *grid = list->data;
    
    GtkWidget *nickEntry        = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0),
              *colorButton      = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1),
              *movesCheckButton = gtk_grid_get_child_at(GTK_GRID(grid), 0, 2);
              
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colorButton), &info->color1);
    
    const gchar* nick = gtk_entry_get_text(GTK_ENTRY(nickEntry));
    
    info->nick1 = malloc((strlen(nick)+1)*sizeof(gchar));
    memcpy(info->nick1, nick, strlen(nick) + 1); //+NUL
    
    if(info->player == 2 && strcmp(nick, info->nick2) == 0) {
        showError("\nDrugi gracz wybrał już twój nick!");
        return;
    }

    info->hints = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(movesCheckButton));
    
    GtkStyleContext *context = gtk_widget_get_style_context(info->widget);
    GdkRGBA color;
    
    gtk_style_context_get_color(context,
                                gtk_style_context_get_state(context),
                                &color);
                                
    if(equalColors(&info->color1, &color)) {
        showError("Musisz zmienić swój kolor, plansza ma identyczny!");
        return; 
    }
    
    if(info->player == 2) {
          
        if(equalColors(&info->color1, &info->color2)) {
            gchar err[100]; 
            sprintf(err, "%s już wybrał twój kolor!", info->nick2);
            showError(err);
            return; 
        }
        
    }   

    gtk_widget_hide(info->widget);

    if(info->player == 1) {
        info->darea = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(info->darea), "");
        
        GdkDisplay *display = gdk_display_get_default();
        GdkMonitor *monitor = gdk_display_get_monitor_at_point(display, 0, 0);
        GdkRectangle geo;
        gdk_monitor_get_geometry(monitor, &geo);
        gtk_widget_set_size_request(info->darea, geo.width/7, geo.height/7); 

        gtk_window_set_position(GTK_WINDOW(info->darea), GTK_WIN_POS_CENTER);
        gtk_container_set_border_width(GTK_CONTAINER(info->darea), 10);

        GtkWidget *label = gtk_label_new("Poczekaj...");
        gtk_container_add(GTK_CONTAINER(info->darea),label);

        g_signal_connect(info->darea, "destroy", G_CALLBACK(destroyWaiting), info);        

        gtk_widget_show_all(info->darea);
    }

    sendHelloMessage(info);

    /* player B starts game */

    if(info->player == 2) initGame(info);
}

void sendHelloMessage(gameInfo *info) {

    gchar hello[50];
    gchar *rgba = gdk_rgba_to_string(&info->color1);
    sprintf(hello,"%dhc%sl%ldn%s",info->player,
                                       rgba,
                                    strlen(info->nick1), info->nick1);

    free(rgba);
    sendMessage(hello);
}

void readHelloMessage(gchar *msg, gameInfo *info) {
    if(msg[1] != 'h') return;
    char  delim[3] = "ln",
         *token    = strtok(msg+3, delim);

    gdk_rgba_parse(&info->color2, token);
    int pos = 5 + strlen(token);
    
    token = strtok(NULL, delim); 
    size_t len = atoi(token);
    pos += strlen(token);

    memcpy(info->nick2, msg+pos, len);
    info->nick2[len] = '\0';
}

bool equalColors(GdkRGBA *color1, GdkRGBA *color2) {
    if(color1 == NULL || color2 == NULL) 
        return false;

    return (color1->red   == color2->red   &&
            color1->green == color2->green &&
            color1->blue  == color2->blue);
}
