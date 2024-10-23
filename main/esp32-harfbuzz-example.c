#include <stdio.h>
#include <pthread.h>
#include <SDL3/SDL.h>
#include <hb.h>
#include <hb-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "bsp/esp-bsp.h"
#include "filesystem.h"

void* sdl_thread(void* args) {
    printf("Initializing SDL3 with HarfBuzz\n");

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return NULL;
    }

    // Create window and renderer
    SDL_Window *window = SDL_CreateWindow("HarfBuzz Hello World", BSP_LCD_H_RES, BSP_LCD_V_RES, 0);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return NULL;
    }

    // Clear the screen with initial color
    SDL_SetRenderDrawColor(renderer, 88, 66, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Initialize the filesystem
    SDL_InitFS();

    // Initialize FreeType
    FT_Library ft_library;
    if (FT_Init_FreeType(&ft_library)) {
        printf("FT_Init_FreeType failed\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return NULL;
    }

    // Load font face
    FT_Face ft_face;
    if (FT_New_Face(ft_library, "/assets/FreeSans.ttf", 0, &ft_face)) {
        printf("FT_New_Face failed\n");
        FT_Done_FreeType(ft_library);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return NULL;
    }

    // Set font size
    FT_Set_Pixel_Sizes(ft_face, 0, 48); // Adjust the pixel size as needed

    // Create HarfBuzz font
    hb_font_t *hb_font = hb_ft_font_create(ft_face, NULL);

    // Set up HarfBuzz buffer
    hb_buffer_t *hb_buffer = hb_buffer_create();
    const char *text = "Harfbuzz";
    hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1);
    hb_buffer_guess_segment_properties(hb_buffer);

    // Shape the text
    hb_shape(hb_font, hb_buffer, NULL, 0);

    // Get glyph information and positions
    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(hb_buffer, &glyph_count);

    // Calculate the size of the text surface
    int width = 0;
    int height = 0;
    int x_min = 0;
    int y_min = 0;

    int pen_x = 0;
    int pen_y = 0;

    // Calculate bounding box
    for (unsigned int i = 0; i < glyph_count; ++i) {
        FT_Load_Glyph(ft_face, glyph_info[i].codepoint, FT_LOAD_DEFAULT);
        FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

        FT_GlyphSlot g = ft_face->glyph;

        int x_offset = (glyph_pos[i].x_offset >> 6);
        int y_offset = (glyph_pos[i].y_offset >> 6);

        int x0 = pen_x + x_offset + g->bitmap_left;
        int y0 = pen_y - y_offset - g->bitmap_top;

        int x1 = x0 + g->bitmap.width;
        int y1 = y0 + g->bitmap.rows;

        if (i == 0 || x0 < x_min) x_min = x0;
        if (i == 0 || y0 < y_min) y_min = y0;
        if (x1 > width) width = x1;
        if (y1 > height) height = y1;

        pen_x += (glyph_pos[i].x_advance >> 6);
        pen_y += (glyph_pos[i].y_advance >> 6);
    }

    width -= x_min;
    height -= y_min;

    // Create a surface to hold the text
    SDL_Surface *text_surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!text_surface) {
        printf("SDL_CreateSurface failed: %s\n", SDL_GetError());
        hb_buffer_destroy(hb_buffer);
        hb_font_destroy(hb_font);
        FT_Done_Face(ft_face);
        FT_Done_FreeType(ft_library);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return NULL;
    }

    // Lock the surface for pixel access
    if (!SDL_LockSurface(text_surface)) {
        printf("SDL_LockSurface failed: %s\n", SDL_GetError());
        SDL_DestroySurface(text_surface);
        hb_buffer_destroy(hb_buffer);
        hb_font_destroy(hb_font);
        FT_Done_Face(ft_face);
        FT_Done_FreeType(ft_library);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return NULL;
    }

    // Clear the surface
    memset(text_surface->pixels, 0, text_surface->pitch * text_surface->h);

    // Render the glyphs onto the surface
    pen_x = -x_min;
    pen_y = -y_min;

    for (unsigned int i = 0; i < glyph_count; ++i) {
        FT_Load_Glyph(ft_face, glyph_info[i].codepoint, FT_LOAD_DEFAULT);
        FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

        FT_Bitmap *bitmap = &ft_face->glyph->bitmap;

        int x = pen_x + (glyph_pos[i].x_offset >> 6) + ft_face->glyph->bitmap_left;
        int y = pen_y - (glyph_pos[i].y_offset >> 6) - ft_face->glyph->bitmap_top;

        Uint8 *pixels = (Uint8 *)text_surface->pixels;
        int pitch = text_surface->pitch;

        for (int row = 0; row < bitmap->rows; ++row) {
            for (int col = 0; col < bitmap->width; ++col) {
                Uint8 value = bitmap->buffer[row * bitmap->pitch + col];
                Uint8 r = 255;
                Uint8 g = 255;
                Uint8 b = 255;
                Uint8 a = value;

                int pixel_x = x + col;
                int pixel_y = y + row;

                if (pixel_x < 0 || pixel_x >= width || pixel_y < 0 || pixel_y >= height)
                    continue;

                int pixel_index = pixel_y * pitch + pixel_x * 4;

                pixels[pixel_index + 0] = r;
                pixels[pixel_index + 1] = g;
                pixels[pixel_index + 2] = b;
                pixels[pixel_index + 3] = a;
            }
        }

        pen_x += (glyph_pos[i].x_advance >> 6);
        pen_y += (glyph_pos[i].y_advance >> 6);
    }

    SDL_UnlockSurface(text_surface);

    // Create texture from the surface
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_DestroySurface(text_surface);

    if (!text_texture) {
        printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        hb_buffer_destroy(hb_buffer);
        hb_font_destroy(hb_font);
        FT_Done_Face(ft_face);
        FT_Done_FreeType(ft_library);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return NULL;
    }

    // Clean up HarfBuzz and FreeType
    hb_buffer_destroy(hb_buffer);
    hb_font_destroy(hb_font);
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);

    // Get text dimensions
    int text_width = width;
    int text_height = height;

    // Initialize position and speed
    float text_x = (BSP_LCD_H_RES - text_width) / 2.0f;
    float text_y = (BSP_LCD_V_RES - text_height) / 2.0f;
    float text_speed_x = 2.0f;
    float text_speed_y = 2.0f;

    int running = 1;
    SDL_Event event;

    printf("Entering main loop...\n");

    // Main loop
    while (running) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }

        // Update text position
        text_x += text_speed_x;
        text_y += text_speed_y;

        // Bounce off edges
        if (text_x <= 0 || text_x + text_width >= BSP_LCD_H_RES) {
            text_speed_x = -text_speed_x;
            text_x += text_speed_x;
        }
        if (text_y <= 0 || text_y + text_height >= BSP_LCD_V_RES) {
            text_speed_y = -text_speed_y;
            text_y += text_speed_y;
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 88, 66, 255, 255);
        SDL_RenderClear(renderer);

        // Render the text texture
        SDL_FRect dst_rect = { text_x, text_y, text_width, text_height };
        SDL_RenderTexture(renderer, text_texture, NULL, &dst_rect);

        // Present the renderer
        SDL_RenderPresent(renderer);

        // Delay to control frame rate
        SDL_Delay(16); // Approximate 60 FPS
    }

    // Clean up
    SDL_DestroyTexture(text_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return NULL;
}

void app_main(void) {
    pthread_t sdl_pthread;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 32768);  // Adjust stack size as needed

    int ret = pthread_create(&sdl_pthread, &attr, sdl_thread, NULL);
    if (ret != 0) {
        printf("Failed to create SDL thread: %d\n", ret);
        return;
    }

    pthread_detach(sdl_pthread);
}
