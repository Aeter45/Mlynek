#include "mlynek.h"

static PipesPtr   pipes;
static GtkWidget *toplevel;

void getMessage(gchar *msg)
{
    getStringFromPipe(pipes, msg, 50);
} 

void sendMessage(gchar *msg)
{
    if(msg == NULL) return;
    sendStringToPipe(pipes, msg);
}

void destroyInfo(gameInfo *info)
{
    if(info->nick1) free(info->nick1);
    if(info->nick2) free(info->nick2);
}

/***************************  dialogs  *****************************/

void showVictory(gameInfo *info, int option)
{
    
    GtkWidget *dialog;
    gchar text[100];

    if(option == 2) {

        GtkWidget *label, 
                  *content_area;

        sprintf(text, "Wygrałeś! %s nie mógł wykonać ruchu! Restart?", info->nick2);
        
        GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
        dialog = gtk_dialog_new_with_buttons("Nowa gra?",
                                             GTK_WINDOW(toplevel),
                                             flags,
                                             "_Tak",
                                             GTK_RESPONSE_ACCEPT,
                                             "_Nie",
                                             GTK_RESPONSE_REJECT,
                                             NULL);
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        label = gtk_label_new(text);
        gtk_container_add(GTK_CONTAINER(content_area), label);

        int result = gtk_dialog_run(GTK_DIALOG(dialog));

        gchar ans[5];
        switch(result) {
            case GTK_RESPONSE_ACCEPT:
                sprintf(ans,"%dr", info->player);
                sendMessage(ans);
                break;
            default:
                sprintf(ans,"%de", info->player);
                sendMessage(ans);
                endGame(info);
                break;
        }
        gtk_widget_destroy(dialog);
    } 
    else {
        if(option == 1) sprintf(text, "Wygrałeś! %s się poddał.", info->nick2);
        else sprintf(text, "Wygrałeś!");
        dialog=gtk_message_dialog_new(GTK_WINDOW(toplevel),
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      GTK_MESSAGE_INFO,
                                      GTK_BUTTONS_CLOSE,"%s",text);
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
        endGame(info);

    } 

    /* gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    endGame(info);*/
}

void showDraw(gameInfo *info)
{
    GtkWidget *dialog;
    gchar text[40];
    //if(option == 1) sprintf(text, "Remis! 3 powtórzone sekwencje." );
    sprintf(text, "Remis! %d ruchów bez młynka :(", NO_MILLS_DRAW);
    dialog=gtk_message_dialog_new(GTK_WINDOW(toplevel),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_CLOSE,"%s",text);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy(dialog);
    endGame(info);
}

void askForRestart(GtkWidget *widget __attribute__((unused)), gameInfo *info) {
    gchar text[5];
    sprintf(text,"%da",info->player);
    sendMessage(text);
}

void showQuestion(gameInfo *info, int option) 
{
    GtkWidget *dialog, *label, *content_area;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
    gchar text[50]; 
    sprintf(text, "%s zapytał Cię o restart gry.", info->nick2);
    dialog = gtk_dialog_new_with_buttons("Restart?",
                                         GTK_WINDOW(toplevel),
                                         flags,
                                         "_Tak",
                                         GTK_RESPONSE_ACCEPT,
                                         "_Nie",
                                         GTK_RESPONSE_REJECT,
                                         NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new(text);
    gtk_container_add(GTK_CONTAINER(content_area), label);

    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    gchar ans[5];
    switch(result) {
        case GTK_RESPONSE_ACCEPT:
            sprintf(ans,"%dy",info->player);
            initInfo(info);
            paintSetup(info->darea, info);
            gtk_widget_queue_draw(info->widget);
            gtk_widget_queue_draw(info->darea);
            break;
        default:
            sprintf(ans,"%dn",info->player);
            if(option == 2) {
                sprintf(ans,"%de",info->player);
                sendMessage(ans);
                gtk_widget_destroy(dialog);
                endGame(info);
                return;
            }
            break;
    }
    sendMessage(ans);
    gtk_widget_destroy(dialog);
}

void showError(char *komunikat)
{
    GtkWidget *dialog;
    dialog=gtk_message_dialog_new (GTK_WINDOW(toplevel),GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"%s",komunikat);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

void surrender(GtkWidget *widget __attribute__((unused)), gameInfo *info) 
{
    gchar msg[3];
    sprintf(msg, "%ds", info->player);
    sendMessage(msg);
    endGame(info);
}

void endGame(gameInfo *info) 
{
    destroyInfo(info);
    closePipes(pipes);
    //gtk_widget_destroy(toplevel);
    gtk_main_quit();
}

/****************************  main  ******************************/

int main(int argc,char* argv[]) 
{

    pipes = initPipes(argc, argv);
    if(pipes == NULL || argc != 2) return 1;
    
    gameInfo info;

    if(strcmp(argv[1], "A") == 0) info.player = 1;
    else info.player = 2;

    coords tab[] = {
        {1,1},{13,1},{25,1},{25,13},{25,25},{13,25},{1,25},{1,13},
        {5,5},{13,5},{21,5},{21,13},{21,21},{13,21},{5,21},{5,13},
        {9,9},{13,9},{17,9},{17,13},{17,17},{13,17},{9,17},{9,13}
    }; 
    
    info.array = tab;
    
    gtk_init(&argc, &argv);
    
    toplevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //initGame(&info);
    initMenu(toplevel, &info);

    gtk_main();
    return 0;
}

gboolean painter(gameInfo *info) {
    paintSetup(info->darea, info);
    gtk_widget_queue_draw(info->widget);
    gtk_widget_queue_draw(info->darea);
    return TRUE;
}

void initInfo(gameInfo *info) {

    info->disc = 0;

    info->board1 = 0;
    info->board2 = 0;

    info->turn = (info->player%2)? TRUE : FALSE;

    info->phase = 1;
    info->silo  = 9;
    info->drag = -1;
    info->surface = NULL;

    info->mill = 0;
    info->nomills = 0;
    info->data = 0;

}

void initGame(gameInfo *info) 
{
    
    if(info->player == 1 && info->darea != NULL) gtk_widget_destroy(info->darea);

    toplevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
             
    GtkWidget *darea  = gtk_drawing_area_new(),
              *fixed  = gtk_fixed_new(),
              *box    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0),
              *frame  = gtk_frame_new(""),
              *label  = gtk_label_new("");

    /* game window size */
    
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_monitor_at_point(display, 0, 0);
    GdkRectangle geo;
    gdk_monitor_get_geometry(monitor, &geo);
 
    guint radiLen      = (7*geo.height/12)/31,
          widthInRadi  = 36,
          heightInRadi = 36;

    g_object_set(darea, "margin", radiLen, NULL);

    guint windowWidth  = widthInRadi*radiLen,
          windowHeight = heightInRadi*radiLen;

    GdkGeometry hints;
    hints.min_width  = (hints.max_width  = windowWidth); 
    hints.min_height = (hints.max_height = windowHeight);

    gtk_window_set_geometry_hints(GTK_WINDOW(toplevel),
                                  NULL,
                                  &hints,
                                  GDK_HINT_MAX_SIZE | GDK_HINT_MIN_SIZE );

    gtk_window_set_position(GTK_WINDOW(toplevel), GTK_WIN_POS_CENTER);
    
    gchar title[30];
    sprintf(title, "Młynek - %s", info->nick1);
    gtk_window_set_title(GTK_WINDOW(toplevel), title);


    gtk_widget_set_size_request(darea, (widthInRadi-2)*radiLen, 26*radiLen); //dł boku
    gtk_widget_set_hexpand(darea, FALSE);
    gtk_widget_set_vexpand(darea, FALSE);

    /* setting contents */

    GtkWidget *surrenderButton = gtk_button_new_with_label("Poddaj się"),
              *restartButton   = gtk_button_new_with_label("Zapytaj się o restart");
 
    gtk_box_pack_start(GTK_BOX(box), restartButton,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), surrenderButton, FALSE, FALSE, 0);

    gtk_widget_set_size_request(frame, 34*radiLen, 4*radiLen);    
    gtk_widget_set_size_request(restartButton, 34*radiLen, radiLen);
    gtk_widget_set_size_request(surrenderButton, 34*radiLen, radiLen);

    gtk_container_add(GTK_CONTAINER(toplevel),fixed);
    gtk_fixed_put(GTK_FIXED(fixed), darea, 0, 0);
    gtk_fixed_put(GTK_FIXED(fixed), frame, radiLen, 28*radiLen);
    gtk_fixed_put(GTK_FIXED(fixed), box, radiLen, 32*radiLen);
        
    gtk_container_add(GTK_CONTAINER(frame), label);

    /* setting game properties */    
    
    initInfo(info);

    info->widget = label; 
    info->darea  = darea;

    /* event handling */

    g_timeout_add(100,
                  ((GSourceFunc) (void (*)(void)) (readMessage)),
                  info);
    
    g_signal_connect (G_OBJECT(darea), "configure-event", 
                      G_CALLBACK(configureCb), info);

    g_signal_connect (G_OBJECT(darea), "draw", 
                      G_CALLBACK(drawCb), info);  

    g_signal_connect (darea, "motion-notify-event",
                      G_CALLBACK (motionNotifyCb), info);

    g_signal_connect (darea, "button-press-event",
                      G_CALLBACK (buttonPressCb), info);

    g_signal_connect (darea, "button-release-event",
                      G_CALLBACK (buttonReleaseCb), info);

    gtk_widget_set_events (darea, gtk_widget_get_events (darea)
                                 | GDK_BUTTON_PRESS_MASK
                                 | GDK_BUTTON_RELEASE_MASK
                                 | GDK_POINTER_MOTION_MASK);

    g_signal_connect(toplevel, "destroy", G_CALLBACK(surrender), info);

    g_signal_connect(surrenderButton, "clicked", G_CALLBACK(surrender), info);

    g_signal_connect(restartButton, "clicked", G_CALLBACK(askForRestart), info);

    gtk_widget_show_all(toplevel);
}

void initMenu(GtkWidget *window, gameInfo *info) 
{
    gtk_window_set_title(GTK_WINDOW(window), "Ustawienia");
      gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);
    
    /* title */

    GtkWidget *titleBox      = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0),
              *titleFrame = gtk_frame_new(NULL),
              *titleLabel = gtk_label_new(NULL);
        
    gtk_container_add(GTK_CONTAINER(titleFrame), titleLabel);
    gtk_box_pack_start(GTK_BOX(titleBox), titleFrame, TRUE, TRUE, 20);
    gtk_box_pack_start(GTK_BOX(box), titleBox, TRUE, TRUE, 0);

    char *markup = "<span size=\"34000\"> MŁYNEK </span>";
    gtk_label_set_markup(GTK_LABEL(titleLabel), markup);

    /* settings */
    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 10);

    GtkWidget *nickLabel = gtk_label_new("Enter name:"),
              *nickEntry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), nickLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), nickEntry, 1, 0, 1, 1);

    gtk_label_set_xalign(GTK_LABEL(nickLabel), 0.0f);

    gtk_entry_set_max_length(GTK_ENTRY(nickEntry), 9);
    gtk_entry_set_alignment(GTK_ENTRY(nickEntry), 0.0f);

    if(info->player%2)  gtk_entry_set_text(GTK_ENTRY(nickEntry), "Gracz A");
    else                gtk_entry_set_text(GTK_ENTRY(nickEntry), "Gracz B");
    
    GdkRGBA color; 
    if(info->player%2) gdk_rgba_parse(&color,"rgb(239,41,41)");
       else            gdk_rgba_parse(&color,"rgb(25,246,236)");

    GtkWidget *colorLabel       = gtk_label_new("Wybierz swój kolor:"),
              *colorButton      = gtk_color_button_new_with_rgba(&color),
              *movesCheckButton = gtk_check_button_new_with_label("Pokaż dostępne ruchy");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(movesCheckButton), TRUE);

    gtk_grid_attach(GTK_GRID(grid), colorLabel, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), colorButton, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), movesCheckButton, 0, 2, 2, 1);
    
    gtk_label_set_xalign(GTK_LABEL(colorLabel), 0.0f);
    gtk_button_set_relief(GTK_BUTTON(colorButton), GTK_RELIEF_NONE);

    GtkWidget *startButton = gtk_button_new_with_label("Graj!");

    if(info->player == 2) {

        GtkAdjustment *adj=gtk_adjustment_new(1.0, 1.0, 5.0, 1.0, 1.0, 1.0);

        GtkWidget *botCheckButton = gtk_check_button_new_with_label("Uruchom bota"),
                  *scaleLabel     = gtk_label_new("Ustaw jego siłę:"),
                  *scale          = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, adj);

        gtk_label_set_xalign(GTK_LABEL(scaleLabel), 0.0f);
        gtk_widget_set_sensitive(scaleLabel, FALSE);

        gtk_widget_set_sensitive(botCheckButton, FALSE);    
        gtk_widget_set_sensitive(startButton, FALSE);
        gtk_widget_set_sensitive(scale, FALSE);

        gtk_grid_attach(GTK_GRID(grid), botCheckButton, 0, 3, 2, 1);
        gtk_grid_attach(GTK_GRID(grid), scaleLabel, 0, 4, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), scale, 1, 4, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), startButton, 0, 5, 2, 1);

    } else gtk_grid_attach(GTK_GRID(grid), startButton, 0, 3, 2, 1);

    /* game info initialization */

    info->nick1 = malloc(21*sizeof(gchar));
    info->nick2 = malloc(21*sizeof(gchar));

    info->widget = window;

    /* event handling */
        
    g_timeout_add(100,
                  ((GSourceFunc) (void (*)(void)) (getHelloMessage)),
                  info);

    g_signal_connect(G_OBJECT(startButton), "clicked",
                     G_CALLBACK(readStartInfo), info);

    g_signal_connect(G_OBJECT(window), "destroy", 
                     G_CALLBACK(surrender), info);
    
    gtk_widget_show_all(window);
}
