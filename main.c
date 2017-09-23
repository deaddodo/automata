#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

typedef struct {
    bool birth[9];
    bool survive[9];
} ruleset_t;

typedef struct cell_t {
    struct cell_t *neighbors[8];
    bool alive;
} cell_t;

typedef struct {
    int width, height;
    cell_t *grid;
    cell_t *next;
} world_t;

typedef struct {
    int x, y;
} point_t;

typedef struct {
    SDL_Renderer *r;
    SDL_Window *w;
    unsigned char s;
} render_t;

static const point_t NEIGHBOR_MASK[8] = {
        {.x=-1,.y=-1},
        {.x= 0,.y=-1},
        {.x= 1,.y=-1},
        {.x=-1,.y= 0},
      //{.x= 0,.y= 0),  -- origin, unneeded but here for clarity
        {.x= 1,.y= 0},
        {.x=-1,.y= 1},
        {.x= 0,.y= 1},
        {.x= 1,.y= 1}
};

static const unsigned char MAP[] = {
#include "spawn.txt"
};

void create_world(world_t *world, int width, int height) {
    // set up the world
    world->width = width;
    world->height = height;
    world->grid = malloc(sizeof(cell_t) * (width * height));
    world->next = malloc(sizeof(cell_t) * (width * height));

    // associate nodes
    for(int y=0;y<world->height;y++) {
        for(int x=0;x<world->width;x++) {
            int p = (y * world->width) + x;
            //printf(" > (%i, %i): %i\n", x, y, p);

            // make the cell "dead"
            world->grid[p].alive = false;

            // associate neighbors
            for (int i = 0; i < 8; i++) {
                point_t mask = NEIGHBOR_MASK[i];
                point_t neighbor = {.x=(x + mask.x), .y=(y + mask.y)};
                neighbor.x = (neighbor.x == -1) ? (world->width - 1) : (neighbor.x == (world->width)) ? 0 : neighbor.x;
                neighbor.y = (neighbor.y == -1) ? (world->height - 1) : (neighbor.y == (world->height)) ? 0 : neighbor.y;

                int np = (neighbor.y * world->width) + neighbor.x;
                world->grid[p].neighbors[i] = &world->grid[np];
                //printf(" >>> (%i, %i): %i\n", neighbor.x, neighbor.y, np);
            }
        }
    }
}

void spawn_world(world_t *world) {
    for(int i=0;i<world->width*world->height;i++) {
        if(MAP[i]) {
            world->grid[i].alive = true;
        }
    }
}

void render_world(world_t *world, render_t *renderer) {
    SDL_Rect cell;
    cell.w = cell.h = renderer->s;

    SDL_SetRenderDrawColor(renderer->r, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer->r);

    for(int i = 0; i < (world->width*world->height); i++) {
        if(world->grid[i].alive == true) {
            cell.x = (i % world->width)*renderer->s;
            cell.y = (i / world->width)*renderer->s;

            printf("x: %i, y: %i\n", cell.x, cell.y);

            SDL_SetRenderDrawColor(renderer->r, 0xff, 0xff, 0xff, 0x00);
            SDL_RenderFillRect(renderer->r, &cell);
        }
    }

    SDL_RenderPresent(renderer->r);

    SDL_Delay(1000/30);
}

void destroy_world(world_t *world) {
    free(world->grid);
    free(world->next);
}

void create_renderer(world_t *world, render_t *renderer) {
    //int wX = (world->width < 640) ? 640 : world->width;
    //int wY = (world->height < 480) ? 480 : world->height;
    renderer->s = 6;
    int wX = world->width * renderer->s;
    int wY = world->height * renderer->s;

    SDL_Init(SDL_INIT_VIDEO);
    renderer->w = SDL_CreateWindow("automata", 200, 200, wX, wY, SDL_WINDOW_BORDERLESS);
    renderer->r = SDL_CreateRenderer(renderer->w, -1, 0);
    //SDL_CreateWindowAndRenderer(wX, wY, 0, &(renderer->w), &(renderer->r));
}

void destroy_renderer(render_t *renderer) {
    SDL_DestroyRenderer(renderer->r);
    SDL_DestroyWindow(renderer->w);
}

// TODO: switch this over to a ping-pong buffer to remove unnecessary copy
void tick(world_t *world, ruleset_t *rules) {
    // create next instance
    for(int i=0;i<(world->width*world->height);i++) {
        int n = 0;
        for(int j=0;j<8;j++) {
            if(world->grid[i].neighbors[j]->alive) n+=1;
        }

        if(world->grid[i].alive && rules->survive[n-1]) {
            world->next[i].alive = true;
        } else if(!(world->grid[i].alive) && rules->birth[n-1]) {
            world->next[i].alive = true;
        } else {
            world->next[i].alive = false;
        }
    }

    // tick next instance into current
    for(int i=0;i<(world->width*world->height);i++) {
        world->grid[i].alive = world->next[i].alive;
    }
}

int main() {
    ruleset_t rules;
    world_t world;
    render_t renderer;
    int iter = 1000;

    // TODO: basic ruleset for now, add alternatives later
    for(int i=0;i<9;i++) {
        rules.birth[i] = (i==2) ? true : false;
    }

    for(int i=0;i<9;i++) {
        rules.survive[i] = (i==1 || i==2) ? true : false;
    }

    create_world(&world, 64, 64);
    spawn_world(&world);
    create_renderer(&world, &renderer);
    for(int i=0;i<iter;i++) {
        render_world(&world, &renderer);
        tick(&world, &rules);
    }
    SDL_Delay(4000);
    destroy_renderer(&renderer);
    destroy_world(&world);

    return 0;
}