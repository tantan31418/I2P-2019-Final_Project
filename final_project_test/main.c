// [main.c]
// this template is provided for the 2D shooter game.

#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <math.h>
#include <string.h>

// If defined, logs will be shown on console and written to file.
// If commented out, logs will not be shown nor be saved.
#define LOG_ENABLED

/* Constants. */

// Frame rate (frame per second)
const int FPS = 60;
// Display (screen) width.
const int SCREEN_W = 800;
// Display (screen) height.
const int SCREEN_H = 600;
// At most 4 audios can be played at a time.
const int RESERVE_SAMPLES = 4;
// Same as:
// const int SCENE_MENU = 1;
// const int SCENE_START = 2;
enum {
    SCENE_MENU = 1,
    SCENE_START = 2,
    SCENE_SETTINGS = 3,
    H_DEAD_GAMEOVER = 4,
    M_DEAD_GAMEOVER = 5,
    SCENE_WIN = 6,
    SCENE_HELP = 7
};

/* Input states */

// The active scene id.
int active_scene;
// Keyboard state, whether the key is down or not.
bool key_state[ALLEGRO_KEY_MAX];
// Mouse state, whether the key is down or not.
// 1 is for left, 2 is for right, 3 is for middle.
bool *mouse_state;
// Mouse position.
int mouse_x, mouse_y;
// TODO: More variables to store input states such as joysticks, ...
bool character_flag = 1;
bool ufo_active = 0;
/* Variables for allegro basic routines. */

ALLEGRO_DISPLAY* game_display;
ALLEGRO_EVENT_QUEUE* game_event_queue;
ALLEGRO_TIMER* game_update_timer;

/* Shared resources*/

ALLEGRO_FONT* font_pirulen_32;
ALLEGRO_FONT* font_pirulen_24;
// TODO: More shared resources or data that needed to be accessed
// across different scenes.

/* Menu Scene resources*/
ALLEGRO_BITMAP* main_img_background;
ALLEGRO_BITMAP* img_settings;
ALLEGRO_BITMAP* img_settings2;
ALLEGRO_SAMPLE* main_bgm;
ALLEGRO_SAMPLE_ID main_bgm_id;

/* Start Scene resources*/
ALLEGRO_BITMAP* start_img_background;
ALLEGRO_BITMAP* start_img_background_red;
ALLEGRO_BITMAP* start_img_plane;
ALLEGRO_BITMAP* start_img_plane2;
ALLEGRO_BITMAP* start_img_enemy;
ALLEGRO_BITMAP* start_img_ufo_enemy;
ALLEGRO_SAMPLE* start_bgm;
ALLEGRO_SAMPLE_ID start_bgm_id;
ALLEGRO_BITMAP* img_bullet;
ALLEGRO_BITMAP* img_bullet2;
ALLEGRO_BITMAP* start_img_iss;
ALLEGRO_BITMAP* start_img_ufo;

/* Game over Scene resources*/
ALLEGRO_BITMAP* human_gameover_background;
ALLEGRO_BITMAP* moon_gameover_background;

/*Settings Scene resources*/
ALLEGRO_BITMAP* settings_choose_char;
ALLEGRO_BITMAP* ll_line;
ALLEGRO_BITMAP* tl_line;
ALLEGRO_BITMAP* m45_line;
ALLEGRO_BITMAP* help_img;

/* Win Scene*/
ALLEGRO_BITMAP* win_astronaut;

typedef struct {
    // The center coordinate of the image.
    float x, y;
    // The width and height of the object.
    float w, h;
    // The velocity in x, y axes.
    float vx, vy;
    // Should we draw this object on the screen.
    bool hidden;
    // The pointer to the object’s image.
    ALLEGRO_BITMAP* img;
    //hit count
    int hit_cnt;
} MovableObject;
void draw_movable_object(MovableObject obj);

#define MAX_ENEMY 6
#define MAX_BULLET 30
#define MAX_TEXT 100
#define MAX_ISS 1

MovableObject plane;
int human_blood=5;
int moon_blood=10;
int score=0;
int iss_cnt=0;
int ufo_blood=100;
MovableObject enemies[MAX_ENEMY];
MovableObject bullets[MAX_BULLET];
MovableObject iss[MAX_ENEMY];
MovableObject ufo;
MovableObject ufo_enemies[MAX_ENEMY];

const float MAX_COOLDOWN = 0.15f;
double last_shoot_timestamp;

/* Declare function prototypes. */

// Initialize allegro5 library
void allegro5_init(void);
// Initialize variables and resources.
// Allows the game to perform any initialization it needs before
// starting to run.
void game_init(void);
// Process events inside the event queue using an infinity loop.
void game_start_event_loop(void);
// Run game logic such as updating the world, checking for collision,
// switching scenes and so on.
// This is called when the game should update its logic.
void game_update(void);
// Draw to display.
// This is called when the game should draw itself.
void game_draw(void);
// Release resources.
// Free the pointers we allocated.
void game_destroy(void);
// Function to change from one scene to another.
void game_change_scene(int next_scene);
// Load resized bitmap and check if failed.
ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h);

bool pnt_in_rect(int px, int py, int x, int y, int w, int h);

/* Event callbacks. */
void on_key_down(int keycode);
void on_mouse_down(int btn, int x, int y);

/* Declare function prototypes for debugging. */

// Display error message and exit the program, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// If the program crashes unexpectedly, you can inspect "log.txt" for
// further information.
void game_abort(const char* format, ...);
// Log events for later debugging, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// You can inspect "log.txt" for logs in the last run.
void game_log(const char* format, ...);
// Log using va_list.
void game_vlog(const char* format, va_list arg);

int main(int argc, char** argv) {
    // Set random seed for better random outcome.
    srand(time(NULL));
    allegro5_init();
    game_log("Allegro5 initialized");
    game_log("Game begin");
    // Initialize game variables.
    game_init();
    game_log("Game initialized");
    // Draw the first frame.
    game_draw();
    game_log("Game start event loop");
    // This call blocks until the game is finished.
    game_start_event_loop();
    game_log("Game end");
    game_destroy();
    return 0;
}

void allegro5_init(void) {
    if (!al_init())
        game_abort("failed to initialize allegro");
    
    // Initialize add-ons.
    if (!al_init_primitives_addon())
        game_abort("failed to initialize primitives add-on");
    if (!al_init_font_addon())
        game_abort("failed to initialize font add-on");
    if (!al_init_ttf_addon())
        game_abort("failed to initialize ttf add-on");
    if (!al_init_image_addon())
        game_abort("failed to initialize image add-on");
    if (!al_install_audio())
        game_abort("failed to initialize audio add-on");
    if (!al_init_acodec_addon())
        game_abort("failed to initialize audio codec add-on");
    if (!al_reserve_samples(RESERVE_SAMPLES))
        game_abort("failed to reserve samples");
    if (!al_install_keyboard())
        game_abort("failed to install keyboard");
    if (!al_install_mouse())
        game_abort("failed to install mouse");
    // TODO: Initialize other addons such as video, ...
    
    // Setup game display.
    game_display = al_create_display(SCREEN_W, SCREEN_H);
    if (!game_display)
        game_abort("failed to create display");
    al_set_window_title(game_display, "I2P(I)_2019 Final Project <108062208>");
    
    // Setup update timer.
    game_update_timer = al_create_timer(1.0f / FPS);
    if (!game_update_timer)
        game_abort("failed to create timer");
    
    // Setup event queue.
    game_event_queue = al_create_event_queue();
    if (!game_event_queue)
        game_abort("failed to create event queue");
    
    // Malloc mouse buttons state according to button counts.
    const unsigned m_buttons = al_get_mouse_num_buttons();
    game_log("There are total %u supported mouse buttons", m_buttons);
    // mouse_state[0] will not be used.
    mouse_state = malloc((m_buttons + 1) * sizeof(bool));
    memset(mouse_state, false, (m_buttons + 1) * sizeof(bool));
    
    // Register display, timer, keyboard, mouse events to the event queue.
    al_register_event_source(game_event_queue, al_get_display_event_source(game_display));
    al_register_event_source(game_event_queue, al_get_timer_event_source(game_update_timer));
    al_register_event_source(game_event_queue, al_get_keyboard_event_source());
    al_register_event_source(game_event_queue, al_get_mouse_event_source());
    // TODO: Register other event sources such as timer, video, ...
    
    // Start the timer to update and draw the game.
    al_start_timer(game_update_timer);
}

void game_init(void) {
    /* Shared resources*/
    font_pirulen_32 = al_load_font("pirulen.ttf", 32, 0);
    if (!font_pirulen_32)
        game_abort("failed to load font: pirulen.ttf with size 32");
    
    font_pirulen_24 = al_load_font("pirulen.ttf", 24, 0);
    if (!font_pirulen_24)
        game_abort("failed to load font: pirulen.ttf with size 24");
    
    /* Menu Scene resources*/
    main_img_background = al_load_bitmap("coverjpg.jpg");
    if (!main_img_background)
        game_abort("failed to load image: coverjpg.jpg");
    
    main_bgm = al_load_sample("S31-Night Prowler.ogg");
    if (!main_bgm)
        game_abort("failed to load audio: S31-Night Prowler.ogg");
    
    img_settings = al_load_bitmap("settings.png");
    if (!img_settings)
        game_abort("failed to load image: settings.png");
    img_settings2 = al_load_bitmap("settings2.png");
    if (!img_settings2)
        game_abort("failed to load image: settings2.png");
    
    /* Start Scene resources*/
    start_img_background = load_bitmap_resized("moonbackground_0.jpg", SCREEN_W, SCREEN_H);
    start_img_background_red = load_bitmap_resized("moonbackground_red.png", SCREEN_W, SCREEN_H);
    
    start_img_plane = al_load_bitmap("lunar_lander.png");
    if (!start_img_plane)
        game_abort("failed to load image: lunar_lander.png");
    start_img_plane2 = al_load_bitmap("telescope.png");
    if (!start_img_plane2)
        game_abort("failed to load image: telescope.png");
    
    start_img_enemy = al_load_bitmap("asteroid_50.png");
    if (!start_img_enemy)
        game_abort("failed to load image: asteroid_50.png");
    start_img_ufo_enemy = al_load_bitmap("ufoenemy.png");
    start_img_ufo = al_load_bitmap("ufo.png");
    
    start_bgm = al_load_sample("mythica.ogg");
    if (!start_bgm)
        game_abort("failed to load audio: mythica.ogg");
    
    img_bullet = al_load_bitmap("lhc_bull.png");
    img_bullet2 = al_load_bitmap("telebull.png");
    if (!img_bullet)
        game_abort("failed to load image: image12.png");
    
    start_img_iss = al_load_bitmap("iss.png");
    
    /* Game Over Scene resources*/
    human_gameover_background = load_bitmap_resized("gameover_human.jpg", SCREEN_W, SCREEN_H);
    moon_gameover_background = al_load_bitmap("moondying.jpg");
    /* Settings Scene resources*/
    settings_choose_char = al_load_bitmap("chosechar.png");
    ll_line = al_load_bitmap("ll_line.png");
    tl_line = al_load_bitmap("tl_line.png");
    m45_line = al_load_bitmap("m45-line.png");
    help_img = al_load_bitmap("help.png");
    /* Win Scene*/
    win_astronaut = al_load_bitmap("winastronaut.png");
    
    // Change to first scene.
    game_change_scene(SCENE_MENU);
}

void game_start_event_loop(void) {
    bool done = false;
    ALLEGRO_EVENT event;
    int redraws = 0;
    while (!done) {
        al_wait_for_event(game_event_queue, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            // Event for clicking the window close button.
            game_log("Window close button clicked");
            done = true;
        } else if (event.type == ALLEGRO_EVENT_TIMER) {
            // Event for redrawing the display.
            if (event.timer.source == game_update_timer)
                // The redraw timer has ticked.
                redraws++;
        } else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            // Event for keyboard key down.
            game_log("Key with keycode %d down", event.keyboard.keycode);
            key_state[event.keyboard.keycode] = true;
            on_key_down(event.keyboard.keycode);
        } else if (event.type == ALLEGRO_EVENT_KEY_UP) {
            // Event for keyboard key up.
            game_log("Key with keycode %d up", event.keyboard.keycode);
            key_state[event.keyboard.keycode] = false;
        } else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            // Event for mouse key down.
            game_log("Mouse button %d down at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
            mouse_state[event.mouse.button] = true;
            on_mouse_down(event.mouse.button, event.mouse.x, event.mouse.y);
        } else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            // Event for mouse key up.
            game_log("Mouse button %d up at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
            mouse_state[event.mouse.button] = false;
        } else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if (event.mouse.dx != 0 || event.mouse.dy != 0) {
                // Event for mouse move.
                game_log("Mouse move to (%d, %d)", event.mouse.x, event.mouse.y);
                mouse_x = event.mouse.x;
                mouse_y = event.mouse.y;
            } else if (event.mouse.dz != 0) {
                // Event for mouse scroll.
                game_log("Mouse scroll at (%d, %d) with delta %d", event.mouse.x, event.mouse.y, event.mouse.dz);
            }
        }
        // TODO: Process more events and call callbacks by adding more
        // entries inside Scene.
        
        // Redraw
        if (redraws > 0 && al_is_event_queue_empty(game_event_queue)) {
            // if (redraws > 1)
            //     game_log("%d frame(s) dropped", redraws - 1);
            // Update and draw the next frame.
            game_update();
            game_draw();
            redraws = 0;
        }
    }
}

void game_update(void) {
    if (active_scene == SCENE_START) {
        plane.vx = plane.vy = 0;
        if (key_state[ALLEGRO_KEY_UP] || key_state[ALLEGRO_KEY_W])
            plane.vy -= 1;
        if (key_state[ALLEGRO_KEY_DOWN] || key_state[ALLEGRO_KEY_S])
            plane.vy += 1;
        if (key_state[ALLEGRO_KEY_LEFT] || key_state[ALLEGRO_KEY_A])
            plane.vx -= 1;
        if (key_state[ALLEGRO_KEY_RIGHT] || key_state[ALLEGRO_KEY_D])
            plane.vx += 1;
        // 0.71 is (1/sqrt(2)).
        plane.y += plane.vy * 4 * (plane.vx ? 0.71f : 1);
        plane.x += plane.vx * 4 * (plane.vy ? 0.71f : 1);
        
        if (plane.x-plane.w/2 < 0)
            plane.x = plane.w/2;
        else if (plane.x+plane.w/2 > SCREEN_W)
            plane.x = SCREEN_W-plane.w/2;
        if (plane.y-plane.h/2 < 0)
            plane.y = plane.h/2;
        else if (plane.y+plane.h/2 > SCREEN_H)
            plane.y = SCREEN_H-plane.h/2;
        
        int i;
        for (i=0;i<MAX_BULLET;i++) {
            if (bullets[i].hidden)
                continue;
            bullets[i].x += bullets[i].vx;
            bullets[i].y += bullets[i].vy;//???
            if (bullets[i].x < 0 || bullets[i].y<0)
                bullets[i].hidden = true;
            //bullet collide enemy
            for (int j=0;j<MAX_ENEMY;j++){
                if(bullets[i].x>enemies[j].x-enemies[j].w/2&&bullets[i].x<enemies[j].x+enemies[j].w/2&&bullets[i].y>enemies[j].y-enemies[j].h/2&&bullets[i].y<enemies[j].y+enemies[j].h/2&&enemies[j].hidden==false){
                    
                    enemies[j].hit_cnt++;
                    bullets[i].hidden=true;
                    game_log("bullet hit enemy at %d %d hit count%d",bullets[i].x,bullets[i].y,enemies[j].hit_cnt);
                    //hit count prints weird numbers but function correctly
                    if(enemies[j].hit_cnt==3){
                        enemies[j].hit_cnt=0;
                        enemies[j].hidden=true;
                        score++;
                        break;
                    }
                }
            }
//            //bullet collide ufo enemy
//            for (int j=0;j<MAX_ENEMY;j++){
//                if(bullets[i].x>ufo_enemies[j].x-ufo_enemies[j].w/2&&bullets[i].x<ufo_enemies[j].x+ufo_enemies[j].w/2&&bullets[i].y>ufo_enemies[j].y-ufo_enemies[j].h/2&&bullets[i].y<ufo_enemies[j].y+ufo_enemies[j].h/2&&ufo_enemies[j].hidden==false){
//
//                    ufo_enemies[j].hit_cnt++;
//                    bullets[i].hidden=true;
//                    game_log("bullet hit ufo enemy at %d %d hit count%d",bullets[i].x,bullets[i].y,ufo_enemies[j].hit_cnt);
//                    //hit count prints weird numbers but function correctly
//                    if(ufo_enemies[j].hit_cnt==3){
//                        ufo_enemies[j].hit_cnt=0;
//                        ufo_enemies[j].hidden=true;
//                        score++;
//                        break;
//                    }
//                }
//            }
            //bullet collide ufo
            if ((!ufo.hidden)&&bullets[i].x > ufo.x - ufo.w / 2 && bullets[i].x < ufo.x + ufo.w / 2 && bullets[i].y > ufo.y - ufo.h / 2 && bullets[i].y < ufo.y + ufo.h / 3){
                
                ufo.hit_cnt++;
                game_log("ufo hit count = %d",ufo.hit_cnt);
                bullets[i].hidden = true;
//                if (ufo.hit_cnt % 3 == 0)
//                {
//                    score++;
//                }
                if (ufo.hit_cnt >= 10)
                {
                    ufo.hidden = true;
                    ufo_active=0;
                    game_change_scene(SCENE_WIN);
                }
            }
        }
        //Update enemy coordinates
        for (i=0;i<MAX_ENEMY;i++) {
            if(!ufo_active){
                if (enemies[i].hidden){
                    if(rand()>20000){
                        enemies[i].hidden=false;
                        enemies[i].x = enemies[i].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - enemies[i].w);
                        enemies[i].y = rand()%41;
                        enemies[i].vy=(rand()%10000)*0.00015;}
                }
                else {
                enemies[i].y += enemies[i].vy;
                //enemy collide plane
                if(plane.x>enemies[i].x-enemies[i].w/2&&plane.x<enemies[i].x+enemies[i].w/2&&plane.y>enemies[i].y-enemies[i].h/2&&plane.y<enemies[i].y+enemies[i].h/2){
                    plane.hit_cnt++;
                    game_log("plane hit count=%d",plane.hit_cnt);
                    plane.hidden=true;
                    plane.x = 400;
                    plane.y = 600;
                    plane.hidden=false;
                    //bug:if stay at home will still repeat count
                    human_blood--;
                    if(human_blood==0){
                        game_log("all human are dead");
                        game_change_scene(H_DEAD_GAMEOVER);
                    }
                    
                }
                //enemy hit sceen down
                if (enemies[i].x > SCREEN_W || enemies[i].y>SCREEN_H){
                    enemies[i].hidden = true;
                    moon_blood--;
                    if(moon_blood==0){
                        game_log("moon is dead");
                        game_change_scene(M_DEAD_GAMEOVER); //temp
                    }
                }
                }
            }
            else{
                enemies[i].hidden = true;
            }
        }
//        //ufo enemy
//        for (i=0;i<MAX_ENEMY;i++) {
//            if(ufo_active){
//                if (ufo_enemies[i].hidden){
//                    if(rand()>20000){
//                        ufo_enemies[i].hidden=false;
//                        ufo_enemies[i].x = ufo.x + (float)rand() / RAND_MAX * ufo.w;
//                        ufo_enemies[i].y = ufo.y;
//                        ufo_enemies[i].vy=(rand()%10000)*0.00015;}
//                }
//                else {
//                    ufo_enemies[i].y += ufo_enemies[i].vy;
//                    //enemy collide plane
//                    if(plane.x>ufo_enemies[i].x-ufo_enemies[i].w/2&&plane.x<ufo_enemies[i].x+ufo_enemies[i].w/2&&plane.y>ufo_enemies[i].y-ufo_enemies[i].h/2&&plane.y<ufo_enemies[i].y+ufo_enemies[i].h/2){
//                        plane.hit_cnt++;
//                        game_log("plane hit count=%d",plane.hit_cnt);
//                        plane.hidden=true;
//                        plane.x = 400;
//                        plane.y = 600;
//                        plane.hidden=false;
//                        //bug:if stay at home will still repeat count
//                        human_blood-=2;
//                        if(human_blood==0){
//                            game_log("all human are dead");
//                            game_change_scene(H_DEAD_GAMEOVER);
//                        }
//
//                    }
//                    //enemy hit sceen down
//                    if (ufo_enemies[i].x > SCREEN_W || ufo_enemies[i].y>SCREEN_H){
//                        ufo_enemies[i].hidden = true;
//                        moon_blood-=2;
//                        if(moon_blood<=0){
//                            game_log("moon is dead");
//                            game_change_scene(M_DEAD_GAMEOVER); //temp
//                        }
//                    }
//                }
//            }
//            else{
//                ufo_enemies[i].hidden = true;
//            }
//        }
        //iss
        for (i = 0;i<MAX_ISS;i++){
            if (score&&score%5==0&&iss[i].hidden){
                if(rand()%100>98){
                    iss[i].hidden = false;
                    iss[i].x = iss[i].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - iss[i].w);
                    iss[i].y = rand()%41;
                    iss[i].vx=0.5;
                    iss[i].vy=(rand()%10000)*0.0001;
                }
            }
            else if (iss[i].hidden==false){
                if(iss[i].x+iss[i].w/2 >= SCREEN_W ||iss[i].x-iss[i].w/2 <=0){
                    iss[i].vx=-1*iss[i].vx;
                }
                iss[i].y+=iss[i].vy;
                iss[i].x+=iss[i].vx;
                
                //iss collide plane
                if(plane.x>iss[i].x-iss[i].w/2&&plane.x<iss[i].x+iss[i].w/2&&plane.y>iss[i].y-iss[i].h/2&&plane.y<iss[i].y+iss[i].h/2){
                    iss[i].hidden=true;
                    iss_cnt++;
                    break;
                }
            }
            if (iss[i].x > SCREEN_W || iss[i].y>SCREEN_H){
                iss[i].hidden = true;
                
            }
        }
        double now = al_get_time();//用get time get time
        if (key_state[ALLEGRO_KEY_SPACE] && now - last_shoot_timestamp >= MAX_COOLDOWN) {
            for (i = 0; i<MAX_BULLET;i++) {
                if (bullets[i].hidden)
                    break;
            }
            if (i < MAX_BULLET) {
                last_shoot_timestamp = now;
                bullets[i].hidden = false;
                bullets[i].x = plane.x;//從那個x射出去
                bullets[i].y = plane.y-(plane.h)/2;//要從中心點減一半高度
            }
        }
        /*ufo*/
        if(ufo.hidden){
            if (iss_cnt>=1&&ufo.hit_cnt==0) {
                ufo.hidden=false;
                ufo_active=1;
//                ufo.hit_cnt=0;
            }
        }
        else{
            ufo.y+=ufo.vy;
        }
        
    }
    else if (active_scene == SCENE_SETTINGS){
        
    }
    else if (active_scene == H_DEAD_GAMEOVER){
        
    }
    else if (active_scene == M_DEAD_GAMEOVER){
        
    }
    
}

void game_draw(void) {
    if (active_scene == SCENE_MENU) {
        al_draw_bitmap(main_img_background, 0, 0, 0);
        //        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTER, "Space Shooter");
        //        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 50, 0, "Press enter key to start");
        
        if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W-48, 10, SCREEN_W-10, 48)){
            //            game_log("pnt in rect draw imgsetting2");
            al_draw_bitmap(img_settings2, SCREEN_W-48, 10, 0);
        }
        else{
            al_draw_bitmap(img_settings, SCREEN_W-48, 10, 0);
            //            game_log("draw imgsetting");
        }
    }
    else if (active_scene == SCENE_START) {
        int i;
        if (moon_blood<=3){
            al_draw_bitmap(start_img_background_red, 0, 0, 0);
        }
        else{
            al_draw_bitmap(start_img_background, 0, 0, 0);
        }
        
        draw_movable_object(plane);
        draw_movable_object(ufo);
        for (i = 0; i < MAX_BULLET; i++)
            draw_movable_object(bullets[i]);
        for (i = 0; i < MAX_ENEMY; i++)
            draw_movable_object(enemies[i]);
        for (i = 0; i < MAX_ISS; i++)
            draw_movable_object(iss[i]);
//        for (i = 0; i < MAX_ENEMY; i++)
//            draw_movable_object(ufo_enemies[i]);
        //human blood bar
        char humanblood[MAX_TEXT]="Human Blood:";
        char buff[MAX_TEXT]="";
        sprintf(buff,"%d", human_blood);
        strcat(humanblood, buff);
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 50, 0, humanblood);
        //score
        char score_text[MAX_TEXT]="Score:";
        char buff2[MAX_TEXT]="";
        sprintf(buff2,"%d", score);
        strcat(score_text, buff2);
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W-200, 30, 0, score_text);
        //mooon blood bar
        char moon_text[MAX_TEXT]="Moon Blood:";
        char buff3[MAX_TEXT]="";
        sprintf(buff3,"%d", moon_blood);
        strcat(moon_text, buff3);
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 100, 0, moon_text);
        //iss
        //        double now_time = al_get_time();
        char iss_text[MAX_TEXT]="ISS:";
        char buff4[MAX_TEXT]="";
        sprintf(buff4,"%d", iss_cnt);
        strcat(iss_text, buff4);
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, 30, 0, iss_text);
    }
    
    
    else if (active_scene == SCENE_SETTINGS) {
        al_draw_bitmap(settings_choose_char, 0, 0, 0);
        
        // lunar lander white line 144 197
        if (pnt_in_rect(mouse_x, mouse_y, 151, 205, 299, 381)){
            al_draw_bitmap(ll_line, 144, 197, 0);
        }
        // telescope 494 205
        else if (pnt_in_rect(mouse_x, mouse_y, 500, 220, 635, 350)){
            al_draw_bitmap(tl_line, 494, 205, 0);
        }
        //help
        else if (pnt_in_rect(mouse_x, mouse_y, 673, 26, 756, 165)){
            al_draw_bitmap(m45_line, 664, 0, 0);
        }
    }
    else if (active_scene == H_DEAD_GAMEOVER){
        al_draw_bitmap(human_gameover_background, 0, 0, 0);
        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H/2, ALLEGRO_ALIGN_CENTER, "YOU ARE DEAD!");
        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H/2+40, ALLEGRO_ALIGN_CENTER, "PRESS ENTER TO TRY AGAIN");
    }
    else if (active_scene == M_DEAD_GAMEOVER){
        al_draw_bitmap(moon_gameover_background, 0, 0, 0);
        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H/2, ALLEGRO_ALIGN_CENTER, "THE MOON IS DESTROYED!");
        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H/2+40, ALLEGRO_ALIGN_CENTER, "PRESS ENTER TO TRY AGAIN");
    }
    else if (active_scene == SCENE_WIN){
        al_draw_bitmap(win_astronaut, 0, 0, 0);
    }
    else if (active_scene == SCENE_HELP){
        al_draw_bitmap(help_img, 0, 0, 0);
    }
    
    al_flip_display();
}

void game_destroy(void) {
    // Destroy everything you have created.
    // Free the memories allocated by malloc or allegro functions.
    // Destroy shared resources.
    al_destroy_font(font_pirulen_32);
    al_destroy_font(font_pirulen_24);
    
    /* Menu Scene resources*/
    al_destroy_bitmap(main_img_background);
    al_destroy_sample(main_bgm);
    al_destroy_bitmap(img_settings);
    al_destroy_bitmap(img_settings2);
    
    /* Start Scene resources*/
    al_destroy_bitmap(start_img_background);
    al_destroy_bitmap(start_img_background_red);
    al_destroy_bitmap(start_img_plane);
    al_destroy_bitmap(start_img_plane2);
    al_destroy_bitmap(start_img_enemy);
    al_destroy_bitmap(start_img_ufo_enemy);
    al_destroy_bitmap(start_img_ufo);
    al_destroy_sample(start_bgm);
    al_destroy_bitmap(img_bullet);
    al_destroy_bitmap(img_bullet2);
    al_destroy_bitmap(start_img_iss);
    al_destroy_timer(game_update_timer);
    al_destroy_event_queue(game_event_queue);
    al_destroy_display(game_display);
    free(mouse_state);
    
    /* Game over Scene resources*/
    al_destroy_bitmap(human_gameover_background);
    al_destroy_bitmap(moon_gameover_background);
    
    /* Settings Scene resources*/
    al_destroy_bitmap(settings_choose_char);
    al_destroy_bitmap(ll_line);
    al_destroy_bitmap(tl_line);
    al_destroy_bitmap(m45_line);
    al_destroy_bitmap(help_img);
    
    /*win scene*/
    al_destroy_bitmap(win_astronaut);
}

void game_change_scene(int next_scene) {
    game_log("Change scene from %d to %d", active_scene, next_scene);
    // TODO: Destroy resources initialized when creating scene.
    if (active_scene == SCENE_MENU) {
        al_stop_sample(&main_bgm_id);
        game_log("stop audio (bgm)");
    } else if (active_scene == SCENE_START) {
        al_stop_sample(&start_bgm_id);
        game_log("stop audio (bgm)");
    }
    active_scene = next_scene;
    // TODO: Allocate resources before entering scene.
    if (active_scene == SCENE_MENU) {
        if (!al_play_sample(main_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_bgm_id))
            game_abort("failed to play audio (bgm)");
    }
    else if (active_scene == SCENE_START) {
        int i;
        //init blood
        moon_blood=10;
        human_blood=5;
        ufo.hit_cnt=0;
        ufo_active=0;
        //init score
        score=0;
        iss_cnt=0;
        
        /*plane*/
        //        plane.img = start_img_plane;
        
        if (character_flag){
            plane.img = start_img_plane;
        }
        else{
            plane.img = start_img_plane2;
        }
        
        plane.x = 400;
        plane.y = 600;
        plane.w = al_get_bitmap_width(plane.img);
        plane.h = al_get_bitmap_height(plane.img);
        plane.hit_cnt=0;
        /*enemy*/
        for (i = 0; i < MAX_ENEMY; i++) {
            enemies[i].img = start_img_enemy;
            enemies[i].w = al_get_bitmap_width(start_img_enemy);
            enemies[i].h = al_get_bitmap_height(start_img_enemy);
            enemies[i].x = enemies[i].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - enemies[i].w);
            enemies[i].y = rand()%41;
            enemies[i].vy=(rand()%10000)*0.00015;
            enemies[i].hit_cnt=0;
        }
//        /*ufo enemy*/
//        for (i = 0; i < MAX_ENEMY; i++) {
//            ufo_enemies[i].img = start_img_ufo_enemy;
//            ufo_enemies[i].w = al_get_bitmap_width(start_img_enemy);
//            ufo_enemies[i].h = al_get_bitmap_height(start_img_enemy);
//            ufo_enemies[i].hit_cnt=0;
//            ufo_enemies[i].hidden=true;
//        }
        /*bullet*/
        for (i=0;i<MAX_BULLET;i++) {
            bullets[i].w = al_get_bitmap_width(img_bullet);
            bullets[i].h = al_get_bitmap_height(img_bullet);
            if (character_flag){
                bullets[i].img = img_bullet;
            }
            else{
                bullets[i].img = img_bullet2;
            }
            bullets[i].vx = 0;
            bullets[i].vy = -3;
            bullets[i].hidden = true;
            bullets[i].hit_cnt=0;
        }
        /*iss*/
        for (i=0;i<MAX_ISS;i++){
            iss[i].img = start_img_iss;
            iss[i].w = al_get_bitmap_width(start_img_iss);
            iss[i].h = al_get_bitmap_height(start_img_iss);
            iss[i].hit_cnt=0;
            iss[i].hidden=true;
            iss[i].x = iss[i].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - iss[i].w);
            iss[i].y = rand()%41;
            iss[i].vy=(rand()%10000)*0.0001;
        }
        /*ufo*/
        ufo.img = start_img_ufo;
        ufo.x = 400;
        ufo.y = 0;
        ufo.w = al_get_bitmap_width(ufo.img);
        ufo.h = al_get_bitmap_height(ufo.img);
        ufo.hit_cnt=0;
        ufo.vy=0.3;
        ufo.hidden = true;
        
        
        if (!al_play_sample(start_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &start_bgm_id))
            game_abort("failed to play audio (bgm)");
    }
}

void on_key_down(int keycode) {
    if (active_scene == SCENE_MENU) {
        if (keycode == ALLEGRO_KEY_ENTER)
            game_change_scene(SCENE_START);
    }
    else if (active_scene == H_DEAD_GAMEOVER) {
        if (keycode == ALLEGRO_KEY_ENTER)
            game_change_scene(SCENE_MENU);
    }
    else if (active_scene == M_DEAD_GAMEOVER) {
        if (keycode == ALLEGRO_KEY_ENTER)
            game_change_scene(SCENE_MENU);
    }
    else if (active_scene == SCENE_WIN) {
        if (keycode == ALLEGRO_KEY_ENTER)
            game_change_scene(SCENE_MENU);
    }
    else if (active_scene == SCENE_HELP) {
        if (keycode == ALLEGRO_KEY_ENTER)
            game_change_scene(SCENE_MENU);
    }
}

void on_mouse_down(int btn, int x, int y) {
    // [HACKATHON 3-8]done
    // TODO: When settings clicked, switch to settings scene.
    // Uncomment and fill in the code below.
    // 從上面的code(event loop那裡) 可以看到會傳mouse event給btn，如果event.mouse.button==1代表左鍵點下去，可參考task5，所以點下去的時候就是要切換到settings
    if (active_scene == SCENE_MENU) {
        if (btn == 1) {
            if (pnt_in_rect(x, y, SCREEN_W-48, 10, SCREEN_W-18, 48))
                game_change_scene(SCENE_SETTINGS);
        }
    }
    else if (active_scene == SCENE_SETTINGS){
        if (btn == 1){
            // lunar lander
            if (pnt_in_rect(x, y, 151, 205, 299, 381)){
                character_flag=1;
                game_change_scene(SCENE_MENU);
            }
            // telescope
            else if (pnt_in_rect(x, y, 500, 220, 635, 350)){
                character_flag=0;
                game_change_scene(SCENE_MENU);
            }
            //help
            else if (pnt_in_rect(x, y, 673, 26, 756, 165)){
                game_change_scene(SCENE_HELP);
            }
        }
    }
}

void draw_movable_object(MovableObject obj) {
    if (obj.hidden)
        return;
    al_draw_bitmap(obj.img, round(obj.x - obj.w / 2), round(obj.y - obj.h / 2), 0);
}

ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h) {
    ALLEGRO_BITMAP* loaded_bmp = al_load_bitmap(filename);
    if (!loaded_bmp)
        game_abort("failed to load image: %s", filename);
    ALLEGRO_BITMAP *resized_bmp = al_create_bitmap(w, h);
    ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();
    
    if (!resized_bmp)
        game_abort("failed to create bitmap when creating resized image: %s", filename);
    al_set_target_bitmap(resized_bmp);
    al_draw_scaled_bitmap(loaded_bmp, 0, 0,
                          al_get_bitmap_width(loaded_bmp),
                          al_get_bitmap_height(loaded_bmp),
                          0, 0, w, h, 0);
    al_set_target_bitmap(prev_target);
    al_destroy_bitmap(loaded_bmp);
    
    game_log("resized image: %s", filename);
    
    return resized_bmp;
}


//bool pnt_in_rect(int px, int py, int x, int y, int w, int h) {
//    return (px>=x && px<=(x+w) && py>=y && py<=(py+h)) ? true:false;
//}
bool pnt_in_rect(int px, int py, int x, int y, int x2, int y2) {
    return (px>=x && px<=x2 && py>=y && py<=y2) ? true:false;
}


// +=================================================================+
// | Code below is for debugging purpose, it's fine to remove it.    |
// | Deleting the code below and removing all calls to the functions |
// | doesn't affect the game.                                        |
// +=================================================================+

void game_abort(const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    game_vlog(format, arg);
    va_end(arg);
    fprintf(stderr, "error occured, exiting after 2 secs");
    // Wait 2 secs before exiting.
    al_rest(2);
    // Force exit program.
    exit(1);
}

void game_log(const char* format, ...) {
#ifdef LOG_ENABLED
    va_list arg;
    va_start(arg, format);
    game_vlog(format, arg);
    va_end(arg);
#endif
}

void game_vlog(const char* format, va_list arg) {
#ifdef LOG_ENABLED
    static bool clear_file = true;
    vprintf(format, arg);
    printf("\n");
    // Write log to file for later debugging.
    FILE* pFile = fopen("log.txt", clear_file ? "w" : "a");
    if (pFile) {
        vfprintf(pFile, format, arg);
        fprintf(pFile, "\n");
        fclose(pFile);
    }
    clear_file = false;
#endif
}
