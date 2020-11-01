// glui.cpp : Defines the entry point for the application.
//
#include "glui.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_IO
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

typedef enum cursor_method_type {
    cursor_sync_query = 0,
    cursor_input_message
};
__inline static
void usage(void)
{
    printf("Usage: inputlag [-h] [-f]\n");
    printf("Options:\n");
    printf("  -f create full screen window\n");
    printf("  -h show this help\n");
}
struct nk_vec2 cursor_new, cursor_pos, cursor_vel;
cursor_method_type cursor_method = cursor_sync_query;

static struct nk_image
icon_load(const char* filename)
{
    int x, y, n;
    GLuint tex;
    unsigned char* data = stbi_load(filename, &x, &y, &n, 0);
    if (!data) printf("[SDL]: failed to load image: %s", filename);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return nk_image_id((int)tex);
}

void sample_input(GLFWwindow* window)
{
    float a = .25; // exponential smoothing factor

    if (cursor_method == cursor_sync_query) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        cursor_new.x = (float)x;
        cursor_new.y = (float)y;
    }

    cursor_vel.x = (cursor_new.x - cursor_pos.x) * a + cursor_vel.x * (1 - a);
    cursor_vel.y = (cursor_new.y - cursor_pos.y) * a + cursor_vel.y * (1 - a);
    cursor_pos = cursor_new;
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    cursor_new.x = (float)xpos;
    cursor_new.y = (float)ypos;
}

int enable_vsync = nk_true;

void update_vsync()
{
    glfwSwapInterval(enable_vsync == nk_true ? 1 : 0);
}

int swap_clear = nk_false;
int swap_finish = nk_true;
int swap_occlusion_query = nk_false;
int swap_read_pixels = nk_false;
GLuint occlusion_query;

void swap_buffers(GLFWwindow* window)
{
    glfwSwapBuffers(window);

    if (swap_clear)
        glClear(GL_COLOR_BUFFER_BIT);

    if (swap_finish)
        glFinish();

    if (swap_occlusion_query) {
        GLint occlusion_result;
        if (!occlusion_query)
            glGenQueries(1, &occlusion_query);
        glBeginQuery(GL_SAMPLES_PASSED, occlusion_query);
        glBegin(GL_POINTS);
        glVertex2f(0, 0);
        glEnd();
        glEndQuery(GL_SAMPLES_PASSED);
        glGetQueryObjectiv(occlusion_query, GL_QUERY_RESULT, &occlusion_result);
    }

    if (swap_read_pixels) {
        unsigned char rgba[4];
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    }
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, 1);
        break;
    }
}

void draw_marker(struct nk_command_buffer* canvas, int lead, struct nk_vec2 pos)
{
    struct nk_color colors[4] = { nk_rgb(255,0,0), nk_rgb(255,255,0), nk_rgb(0,255,0), nk_rgb(0,96,255) };
    struct nk_rect rect = { -5 + pos.x, -5 + pos.y, 10, 10 };
    nk_fill_circle(canvas, rect, colors[lead]);
}

int main(int argc, char** argv)
{
    int ch, width, height;
    unsigned long frame_count = 0;
    double last_time, current_time;
    double frame_rate = 0;
    int fullscreen = GLFW_FALSE;
    GLFWmonitor* monitor = NULL;
    GLFWwindow* window;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    int show_forecasts = nk_true;

    while ((ch = getopt(argc, argv, "fh")) != -1)
    {
        switch (ch)
        {
        case 'h':
            usage();
            exit(EXIT_SUCCESS);

        case 'f':
            fullscreen = GLFW_TRUE;
            break;
        }
    }

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (fullscreen)
    {
        const GLFWvidmode* mode;

        monitor = glfwGetPrimaryMonitor();
        mode = glfwGetVideoMode(monitor);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = 640;
        height = 480;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "Input lag test", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    update_vsync();

    last_time = glfwGetTime();

    nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas* atlas;
        nk_glfw3_font_stash_begin(&atlas);
        struct nk_font_config cfg = nk_font_config(28);
        //cfg.merge_mode = nk_true;
        cfg.range = nk_font_chinese_glyph_ranges();
        //cfg.coord_type = NK_COORD_PIXEL;
        struct nk_font* font = nk_font_atlas_add_from_file(atlas, "fonts/msyh.ttf", 12, &cfg);
        nk_glfw3_font_stash_end();
        if (font != nullptr)
        {
            nk_style_set_font(nk, &font->handle);
        }
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    auto homeImg = icon_load("icons/home.png");
    static char field_buffer_data[64] = { 0 };
    static int field_buffer_size = 0;
    static const int field_buffer_size_max = 64;
    memset(field_buffer_data, 0, sizeof(field_buffer_data));

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        struct nk_rect area;

        glfwPollEvents();
        sample_input(window);

        glfwGetWindowSize(window, &width, &height);
        area = nk_rect(0.f, 0.f, (float)width, (float)height);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_new_frame();
        if (nk_begin(nk, "", area, 0))
        {
            nk_flags align_left = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE;
            struct nk_command_buffer* canvas = nk_window_get_canvas(nk);
            int lead;

            for (lead = show_forecasts ? 3 : 0; lead >= 0; lead--)
                draw_marker(canvas, lead, nk_vec2(cursor_pos.x + cursor_vel.x * lead,
                    cursor_pos.y + cursor_vel.y * lead));

            // print instructions
            nk_layout_row_dynamic(nk, 20, 1);
            nk_label(nk, "Move mouse uniformly and check marker under cursor:", align_left);
            for (lead = 0; lead <= 3; lead++) {
                nk_layout_row_begin(nk, NK_STATIC, 12, 2);
                nk_layout_row_push(nk, 25);
                draw_marker(canvas, lead, nk_layout_space_to_screen(nk, nk_vec2(20, 5)));
                nk_label(nk, "", 0);
                nk_layout_row_push(nk, 500);
                if (lead == 0)
                    nk_label(nk, "- current cursor position (no input lag)", align_left);
                else
                    nk_labelf(nk, align_left, "- %d-frame forecast (input lag is %d frame)", lead, lead);
                nk_layout_row_end(nk);
            }

            nk_layout_row_dynamic(nk, 20, 1);

            nk_checkbox_label(nk, "Show forecasts", &show_forecasts);
            nk_label(nk, "Input method:", align_left);
            if (nk_option_label(nk, "glfwGetCursorPos (sync query)", cursor_method == cursor_sync_query))
                cursor_method = cursor_sync_query;
            if (nk_option_label(nk, "glfwSetCursorPosCallback (latest input message)", cursor_method == cursor_input_message))
                cursor_method = cursor_input_message;

            nk_label(nk, "", 0); // separator

            nk_value_float(nk, "FPS", (float)frame_rate);
            if (nk_checkbox_label(nk, "Enable vsync", &enable_vsync))
                update_vsync();

            nk_label(nk, "", 0); // separator

            nk_label(nk, "After swap:", align_left);
            nk_checkbox_label(nk, "glClear", &swap_clear);
            nk_checkbox_label(nk, "glFinish", &swap_finish);
            nk_checkbox_label(nk, "draw with occlusion query", &swap_occlusion_query);
            nk_checkbox_label(nk, "glReadPixels", &swap_read_pixels);

            nk_layout_row_static(nk, 32, 32, 1);
            bool is_clicked = nk_false;
            const struct nk_input* in;
            auto state = nk_widget(&area, nk);
            if (!state) return 0;
            in = ((state == NK_WIDGET_ROM) || nk->current->layout->flags & NK_WINDOW_ROM) ? 0 : &nk->input;
            if (nk_do_button_image(&nk->last_widget_state, &nk->current->buffer, area,
                homeImg, NK_BUTTON_DEFAULT, &nk->style.button, in))
            {
                is_clicked = nk_true;
                printf("is_clicked=%d\n", is_clicked);
            }
            nk_layout_row_static(nk, 32, 80, 1);
            nk_label(nk, "input text:", NK_TEXT_LEFT);
            nk_layout_row_static(nk, 32, 160, 1);
            nk_edit_string(nk, NK_EDIT_SIMPLE, field_buffer_data, &field_buffer_size, field_buffer_size_max, nk_filter_default);
            nk_layout_row_static(nk, 72, 72, 1);
            nk_image(nk, homeImg);
        }

        nk_end(nk);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        swap_buffers(window);

        frame_count++;

        current_time = glfwGetTime();
        if (current_time - last_time > 1.0)
        {
            frame_rate = frame_count / (current_time - last_time);
            frame_count = 0;
            last_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}