#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <time.h>

#include <SFML/Graphics.h>
#include <SFML/Window.h>

#include "msp.h"
#include "msp_displayport.h"
#include "serial.h"
#include "network.h"

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#define OSD_WIDTH 31
#define OSD_HEIGHT 16

#define X_OFFSET 180

#define PORT 7654

#define WIDTH 1440
#define HEIGHT 810

#define FONT_WIDTH 36
#define FONT_HEIGHT 54

static volatile sig_atomic_t quit = 0;
sfTexture *font;
sfSprite *font_sprite;
sfRenderWindow *window;
uint8_t character_map[OSD_WIDTH][OSD_HEIGHT];
displayport_vtable_t *display_driver;

static void sig_handler(int _)
{
    quit = 1;
}

static void draw_character(uint32_t x, uint32_t y, uint8_t c)
{
    if (x > OSD_WIDTH - 1 || y > OSD_HEIGHT - 1)
    {
        return;
    }
    character_map[x][y] = c;
}

static void draw_screen()
{
    sfRenderWindow_clear(window, sfColor_fromRGB(55, 55, 55));
    for (int y = 0; y < OSD_HEIGHT; y++)
    {
        for (int x = 0; x < OSD_WIDTH; x++)
        {
            uint8_t c = character_map[x][y];
            if (c != 0)
            {
                DEBUG_PRINT("%c", c > 31 ? c : 20);
                sfIntRect r = {0, FONT_HEIGHT * c, FONT_WIDTH, FONT_HEIGHT};
                sfVector2f dest = {(x * FONT_WIDTH) + X_OFFSET, y * FONT_HEIGHT};
                sfSprite_setTextureRect(font_sprite, r);
                sfSprite_setPosition(font_sprite, dest);
                sfRenderWindow_drawSprite(window, font_sprite, NULL);
            }
            DEBUG_PRINT(" ");
        }
        DEBUG_PRINT("\n");
    }
}

static void clear_screen()
{
    DEBUG_PRINT("clear\n");
    memset(character_map, 0, sizeof(character_map));
}

static void draw_complete()
{
    draw_screen();
    sfRenderWindow_display(window);
    DEBUG_PRINT("draw complete!\n");
}

static void msp_callback(msp_msg_t *msp_message)
{
    displayport_process_message(display_driver, msp_message);
}

int main(int argc, char *args[])
{
    struct pollfd poll_fds[1];
    signal(SIGINT, sig_handler);
    memset(character_map, 0, sizeof(character_map));
    sfVideoMode videoMode = {1440, 810, 32};
    window = sfRenderWindow_create(videoMode, "MSP OSD", 0, NULL);
    sfRenderWindow_display(window);
    font = sfTexture_createFromFile("bold.png", NULL);
    font_sprite = sfSprite_create();
    sfSprite_setTexture(font_sprite, font, 0);

    display_driver = calloc(1, sizeof(displayport_vtable_t));
    display_driver->draw_character = &draw_character;
    display_driver->clear_screen = &clear_screen;
    display_driver->draw_complete = &draw_complete;

    msp_state_t *msp_state = calloc(1, sizeof(msp_state_t));
    msp_state->cb = &msp_callback;

    int socket_fd = bind_socket(PORT);
    int recv_len = 0;
    uint8_t buffer[4096];

    struct timespec fps_start, now;
    uint32_t message_counter = 0;
    clock_gettime(CLOCK_MONOTONIC, &fps_start);

    printf("started up, listening on port %d\n", PORT);

    while (!quit)
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        if(now.tv_sec > fps_start.tv_sec) {
            clock_gettime(CLOCK_MONOTONIC, &fps_start);
            printf("Got %d messages in the last second\n", message_counter);
            message_counter = 0;
        }        

        sfEvent event;
        sfRenderWindow_pollEvent(window, &event);

        // Close window: exit
        if (event.type == sfEvtMouseButtonReleased)
        {
            sfRenderWindow_close(window);
            quit = 1;
        }

        poll_fds[0].fd = socket_fd;
        poll_fds[0].events = POLLIN;
        if(poll(poll_fds, 1, 50) > 0) // poll every 50ms so we also go back through the SFML loop  
        {
            struct sockaddr_storage src_addr;
            socklen_t src_addr_len=sizeof(src_addr);
            if (0 < (recv_len = recvfrom(socket_fd,&buffer,sizeof(buffer),0,(struct sockaddr*)&src_addr,&src_addr_len)))
            {
                message_counter++;
                for (int i=0; i<recv_len; i++) 
                    msp_process_data(msp_state, buffer[i]);
            }
        }
    }
    sfRenderWindow_close(window);
    sfSprite_destroy(font_sprite);
    sfTexture_destroy(font);
    sfRenderWindow_destroy(window);
    free(msp_state);
    free(display_driver);
    return 0;
}