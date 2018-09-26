#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"
#include "text.h"
#include "sprites.h"
#include "trig.h"
#include "images.h"

Player player;
Position center;

Asteroid asteroids[NUM_ASTEROIDS];
short asteroidIdx = 0;

Bullet bullets[NUM_BULLETS];
short bulletIdx = 0;

// rotation states for asteroids
RotationState rotations[NUM_ROTATIONS];

short asteroidCount = 3;

State currentState = START;

int main()
{
    REG_DISPCNT = MODE3 | BG2_ENABLE | OBJ_ENABLE | SPRITES_DIMENSION_TYPE;

    init_sprites();
    init_game();

    unsigned int prevButtons = 0;
    while (true) {
        waitForVblank();

        // edge detection for buttons
        unsigned int risingEdge = ~BUTTONS & prevButtons;
        prevButtons = BUTTONS;

        rand(); // because rand() always has the same seed

        switch (currentState)
        {
            case START:
            case START_NODRAW:
                if (risingEdge)
                {
                    currentState = INGAME;
                    start_game();
                }
                break;
            case INGAME:
                if (risingEdge & BUTTON_SELECT)
                {
                    currentState = START;
                    stop_game();
                }
                break;
            case GAMEOVER:
            case GAMEOVER_NODRAW:
                if (risingEdge)
                {
                    currentState = START;
                }
                break;
        }
        switch (currentState)
        {
            case START:
                drawImage(0, 0, TITLE_WIDTH, TITLE_HEIGHT, title);
                currentState = START_NODRAW;
                break;
            case INGAME:
                tick();
                break;
            case GAMEOVER:
                drawImage(0, 0, GAMEOVER_WIDTH, GAMEOVER_HEIGHT, gameover);
                char score[10];
                int len = sprintf(score, "%i", player.score);
                drawCenteredString(45, 60, score, len);
                currentState = GAMEOVER_NODRAW;
                break;
            default: break;
        }
        flush_sprites();
    }
}

void tick()
{
    // draw score
    char score[10];
    int len = sprintf(score, "%i", player.score);
    drawCenteredString(WIDTH >> 1, 4, score, len);

    update_rotations();

    if (player.alive)
    {
        // cache trig values
        player.sin = sin(player.theta);
        player.cos = cos(player.theta);

        if (KEY_DOWN_NOW(BUTTON_UP))
        {
            player.pos.dx -= player.sin << 3;
            player.pos.dy -= player.cos << 3;

            // engine on sprite
            spriteBuffer->attr2 = SHIP1_PALETTE | SHIP1_ID;
            
            // limit player speed
            if (sq(player.pos.dx >> 16) + sq(player.pos.dy >> 16) > 5)
            {
                player.pos.dx -= player.pos.dx >> 4;
                player.pos.dy -= player.pos.dy >> 4;
            }
        } else {
            // engine off sprite
            spriteBuffer->attr2 = SHIP0_PALETTE | SHIP0_ID;
        }
        update_pos(&player.pos);

        // firing delay
        if (player.cooldown > 0)
            player.cooldown--;
        if (KEY_DOWN_NOW(BUTTON_A) && player.cooldown == 0)
        {
            fire_bullet();
        }

        // rotation controls
        if (KEY_DOWN_NOW(BUTTON_LEFT))
        {
            player.theta += 3;
        }
        if (KEY_DOWN_NOW(BUTTON_RIGHT))
        {
            player.theta -= 3;
        }
        if (player.theta >= 360)
        {
            player.theta -= 360;
        }
        if (player.theta < 0)
        {
            player.theta += 360;
        }

        // update player sprite position
        spriteBuffer->attr0 = (((player.pos.y >> 16) - 16) & ATTR0_Y) | SPRITES_PALETTE_TYPE | SHIP0_SPRITE_SHAPE | ATTR0_AFF_DBL;
        spriteBuffer->attr1 = (((player.pos.x >> 16) - 8) & ATTR1_X) | SHIP0_SPRITE_SIZE | AFFINE_ID(0);

        // update player sprite rotation
        aff_matrix(affineBuffer, player.cos, -player.sin, player.sin, player.cos);
    }
    else if (player.explosionTicks > 0)
    {
        // use explosion sprite
        player.explosionTicks--;
        spriteBuffer->attr2 = SHIP2_PALETTE | SHIP2_ID;
    }
    else
    {
        // hide player sprite
        spriteBuffer->attr0 = ATTR0_HIDE;
    }

    for (int i = 0; i < NUM_BULLETS; i++)
    {
        Bullet *bullet = &bullets[i];
        ObjAttr *sprite = &spriteBuffer[i + 1];
        if (bullet->enabled)
        {
            // despawn timer
            if (bullet->age++ >= 30)
            {
                bullet->enabled = false;
            }

            Position *pos = &bullet->pos;
            update_pos(pos);

            // update bullet sprite
            sprite->attr0 = pos->y >> 16 | SPRITES_PALETTE_TYPE | BULLET_SPRITE_SHAPE;
            sprite->attr1 = pos->x >> 16 | BULLET_SPRITE_SIZE;
            sprite->attr2 = BULLET_PALETTE | BULLET_ID;
        }
        else
        {
            sprite->attr0 = ATTR0_HIDE;
        }
    }

    char cleared = true;        // player has destroyed all asteroids
    player.canRespawn = true;   // respawn position is safe
    for (int i = 0; i < NUM_ASTEROIDS; i++)
    {
        Asteroid *asteroid = &asteroids[i];
        ObjAttr *sprite = &spriteBuffer[i + 6];
        char collision = false;
        if (asteroid->enabled)
        {
            cleared = false;
            Position *pos = &asteroid->pos;
            update_pos(pos);

            int r = asteroid_radius(asteroid);
            if (player.alive)
            {
                // player collision
                if (dist_sq(pos, &player.pos) < sq(r + 3))
                {
                    collision = true;
                    player.alive = false;
                    player.explosionTicks = 20;
                    player.lives--;
                    // cover one of the life indicators with a black rectangle :^)))
                    clear_rect(16 - 8 * player.lives, 11, 8, 8);
                }
            }
            else
            {
                // check respawn point
                if (dist_sq(pos, &center) < sq(r + 18))
                {
                    player.canRespawn = false;
                }
            }

            // bullet collision
            for (int j = 0; j < NUM_BULLETS && !collision; j++)
            {
                Bullet *bullet = &bullets[j];
                if (bullet->enabled && dist_sq(pos, &bullet->pos) < sq(r))
                {
                    collision = true;
                    bullet->enabled = false;
                }
            }

            if (collision)
            {
                // split asteroid
                player.score += (3 - asteroid->size) * 50;
                asteroid->enabled = false;
                if (asteroid->size != SMALL)
                    spawn_asteroids(pos, 2, asteroid->size - 1, r);
            }

            // update asteroid sprite
            switch (asteroid->size)
            {
                case SMALL:
                    sprite->attr0 = (((pos->y >> 16) - 8) & ATTR0_Y) | SPRITES_PALETTE_TYPE | ASTEROID3_SPRITE_SHAPE | ATTR0_AFF_DBL;
                    sprite->attr1 = (((pos->x >> 16) - 8) & ATTR1_X) | ASTEROID3_SPRITE_SIZE | AFFINE_ID(asteroid->affine + 1);
                    sprite->attr2 = ASTEROID3_PALETTE | ASTEROID3_ID;
                    break;
                case MEDIUM:
                    sprite->attr0 = (((pos->y >> 16) - 16) & ATTR0_Y) | SPRITES_PALETTE_TYPE | ASTEROID2_SPRITE_SHAPE | ATTR0_AFF_DBL;
                    sprite->attr1 = (((pos->x >> 16) - 16) & ATTR1_X) | ASTEROID2_SPRITE_SIZE | AFFINE_ID(asteroid->affine + 1);
                    sprite->attr2 = ASTEROID2_PALETTE | ASTEROID2_ID;
                    break;
                case LARGE:
                    sprite->attr0 = (((pos->y >> 16) - 32) & ATTR0_Y) | SPRITES_PALETTE_TYPE | ASTEROID1_SPRITE_SHAPE | ATTR0_AFF_DBL;
                    sprite->attr1 = (((pos->x >> 16) - 32) & ATTR1_X) | ASTEROID1_SPRITE_SIZE | AFFINE_ID(asteroid->affine + 1);
                    sprite->attr2 = ASTEROID1_PALETTE | ASTEROID1_ID;
                    break;
            }
        }
        else
        {
            sprite->attr0 = ATTR0_HIDE;
        }
    }

    if (!player.alive && player.explosionTicks == 0)
    {
        // respawn or game over
        try_respawn();
    }
    if (cleared && player.alive)
    {
        // spawn more asteroids if level cleared
        reset_level();
    }
}

void init_game()
{
    center.x = (WIDTH << 15);
    center.y = (HEIGHT << 15);

    // setup player sprite
    spriteBuffer->attr0 = SPRITES_PALETTE_TYPE | SHIP0_SPRITE_SHAPE | ATTR0_AFF_DBL;
    spriteBuffer->attr1 = SHIP0_SPRITE_SIZE | AFFINE_ID(0);

    // init asteroid rotation states
    for (int i = 0; i < NUM_ROTATIONS; i++)
    {
        rotations[i].theta = (i >> 1) * 360 / NUM_ROTATIONS;
        rotations[i].rate = i % 2 == 0 ? -1 : 1;
    }
    //aff_matrix(affineBuffer + 1, 1 << 8, 0, 0, 1 << 8);
}

void update_rotations()
{
    for (int i = 0; i < NUM_ROTATIONS; i++)
    {
        aff_rotation(&affineBuffer[i + 1], rotations[i].theta);
        rotations[i].theta += rotations[i].rate;
        if (rotations[i].theta < 0)
        {
            rotations[i].theta += 360;
        }
        else if (rotations[i].theta >= 360)
        {
            rotations[i].theta -= 360;
        }
    }
}

void start_game()
{
    clear_screen();
    // draw lives image
    drawImage(0, 0, LIVES_WIDTH, LIVES_HEIGHT, lives);

    player.score = 0;
    player.cooldown = 0;
    player.pos = center;
    player.theta = 0;
    player.alive = true;
    player.lives = 3;
    bulletIdx = 0;
    asteroidIdx = 0;
    asteroidCount = 3;

    // reset entities
    for (int i = 0; i < NUM_BULLETS; i++)
    {
        bullets[i].enabled = false;
    }
    for (int i = 0; i < NUM_ASTEROIDS; i++)
    {
        asteroids[i].enabled = false;
    }
    spawn_asteroids(&player.pos, asteroidCount, LARGE, 75);
}

void reset_level()
{
    player.pos.dx = 0;
    player.pos.dy = 0;
    asteroidIdx = 0;
    if (asteroidCount < 10)
        ++asteroidCount;
    spawn_asteroids(&player.pos, asteroidCount, LARGE, 75);
}

void stop_game()
{
    for (int i = 0; i < NUM_SPRITES; i++)
        spriteBuffer[i].attr0 = ATTR0_HIDE;
}

void fire_bullet()
{
    Bullet *bullet = &bullets[bulletIdx];
    if (++bulletIdx >= NUM_BULLETS)
        bulletIdx = 0;
    bullet->pos.x = player.pos.x;
    bullet->pos.y = player.pos.y;
    bullet->pos.dx = player.pos.dx - (player.sin << 10);
    bullet->pos.dy = player.pos.dy - (player.cos << 10);
    bullet->age = 0;
    bullet->enabled = true;
    player.cooldown = 10;
}

void try_respawn()
{
    if (player.lives > 0)
    {
        if (player.canRespawn)
        {
            player.alive = true;
            player.pos = center;
            player.cooldown = 0;
            player.theta = 0;
        }
    }
    else
    {
        stop_game();
        currentState = GAMEOVER;
    }
}

int asteroid_radius(Asteroid *asteroid)
{
    switch (asteroid->size)
    {
        case SMALL: return 4;
        case MEDIUM: return 8;
        case LARGE: return 16;
        default: return -1;
    }
}

/*
 * spawn asteroids along random points on a circle
 */
void spawn_asteroids(Position *pos, int count, int size, int radius)
{
    for (int i = 0; i < count; i++)
    {
        Asteroid *asteroid = &asteroids[asteroidIdx];
        int theta = rand() % 360;
        asteroid->pos.x = pos->x + (cos(theta) << 8) * radius;
        asteroid->pos.y = pos->y + (sin(theta) << 8) * radius;
        theta = rand() % 360;
        asteroid->pos.dx = cos(theta) << (7 - size);
        asteroid->pos.dy = sin(theta) << (7 - size);
        asteroid->size = size;
        asteroid->enabled = true;
        asteroid->affine = rand() % NUM_ROTATIONS;
        asteroidIdx++;
    }
}

int dist_sq(Position *pos1, Position *pos2)
{
    return sq((pos1->x >> 16) - (pos2->x >> 16)) + sq((pos1->y >> 16) - (pos2->y >> 16));
}

/*
 * tick position and handle screen looping
 */
void update_pos(Position *pos)
{
    pos->x += pos->dx;
    if (pos->x < 0)
    {
        pos->x += WIDTH << 16;
    }
    else if (pos->x > WIDTH << 16)
    {
        pos->x -= WIDTH << 16;
    }

    pos->y += pos->dy;
    if (pos->y < 0)
    {
        pos->y += HEIGHT << 16;
    }
    else if (pos->y > HEIGHT << 16)
    {
        pos->y -= HEIGHT << 16;
    }
}

