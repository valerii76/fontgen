/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/28/2012 09:40:00 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Valery Volgutov (Valery Volgutov), valerii76@gmail.com
 *        Company:  3VSoft
 *
 * =====================================================================================
 */

#include "config.h"
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <freetype/ftglyph.h>

void fontgen_log (char const *format, ...)
{
    va_list args;
    va_start (args, format);

    vprintf (format, args);
    /*printf ("\n");*/

    va_end (args);
}

#define TO_STR(x) #x

#define CHECK_FT(exp) \
{\
    FT_Error err = (exp);\
    if (err) \
    {\
        printf (TO_STR (FreeType error <<exp>>: 0x%x\n), err);\
        return -1;\
    }\
}

FT_Library g_library;
FT_Face g_face;

struct cmd_opts
{
    char *font_path;
    int font_size;
    char *sample;
    char *name;

    char display: 1;
    char rotate: 1;
    char progmem: 1;
};

struct font_glyph
{
    unsigned char code;
    unsigned char size;
    int data;
};

int compare (const void* a, const void *b)
{
    return (*(char*) a - *(char*) b);
}

void print_help (void)
{
    printf ("Program using for generate font data for given font and font sample\n");

    printf ("\nusage: fontgen [options] <font path> <size> <sample>\n");
    printf ("options:\n"
            "\t--version:\tprogram version\n"
            "\t--progmem:\tadd PROGMEM for arrays\n"
            "\t--name:\t name for generated array\n"
            "\t-d --display:\tdisplay sample to stdout\n"
            "\t-r --rotate:\trotate font data\n");
}

int parse_cmd (int argc, char *argv[], struct cmd_opts *opts)
{
    int i = 1;
    int req_params = 0;
    int wait_param = 0;

    memset (opts, 0, sizeof (struct cmd_opts));

    for (; i < argc; ++i)
    {
        if (!req_params &&
            (!strcmp (argv [i], "--name") || wait_param == 1))
        {
            if (wait_param)
            {
                opts->name = argv [i];
                wait_param = 0;
            }
            else
                wait_param = 1;
        }
        else if (!req_params &&
                 (!strcmp (argv [i], "--progmem")))
        {
            opts->progmem = 1;
        }
        else if (!req_params &&
                 (!strcmp (argv [i], "-d") || !strcmp (argv [i], "--display")))
        {
            opts->display = 1;
        }
        else if (!req_params &&
                 (!strcmp (argv [i], "-r") || !strcmp (argv [i], "--rotate")))
        {
            opts->rotate = 1;
        }
        else if (!strcmp (argv [i], "--version"))
        {
            printf ("fontgen: version %d.%d\n", MAJOR_VERSION, MINOR_VERSION);
            return 0;
        }
        else if (req_params == 0)
        {
            opts->font_path = argv [i];
            ++req_params;
        }
        else if (req_params == 1)
        {
            opts->font_size = atoi (argv [i]);
            if (opts->font_size == 0)
            {
                printf ("invalide font size: %s\n", argv[i]);
                print_help ();
                return 0;
            }
            ++req_params;
        }
        else if (req_params == 2)
        {
            opts->sample = argv [i];
            ++req_params;
        }
    }

    if (req_params != 3)
    {
        printf ("invalid command line\n");
        print_help ();
        return 0;
    }

    return 1;
}

char const* to_upper (char const *str)
{
    static char g_str[1024];
    int count = strlen (str) + 1; /* add NULL terminated */
    for (; count--;)
    {
        g_str[count] = (char) toupper (str [count]);
    }
    return g_str;
}

int main (int argc, char *argv[])
{
    int i, j, k;
    FT_Bitmap *bitmap = NULL;
    FT_BitmapGlyph bm_glyph;
    FT_Glyph glyph;
    FT_Int index;

    int total_bytes = 0;
    int font_bytes = 0;
    int sample_len;
    struct cmd_opts options;
    struct font_glyph *glyphs = NULL;
    int glyph_count = 0;
    char const *font_name = NULL;


    if (!parse_cmd (argc, argv, &options))
    {
        return 1;
    }

    sample_len = strlen (options.sample);

    qsort (options.sample, sample_len, sizeof (char), compare);

    CHECK_FT (FT_Init_FreeType (&g_library));
    CHECK_FT (FT_New_Face (g_library, options.font_path, 0, &g_face));

    CHECK_FT (FT_Set_Char_Size (g_face, 0, options.font_size << 6, 300, 300));
    CHECK_FT (FT_Set_Pixel_Sizes (g_face, 0, options.font_size));

    font_name = options.name ? options.name : g_face->family_name;

    if (!options.display)
    {
        /* generate header */
        fontgen_log ("/* autogenerated fontgen (Valery Volgutov) */\n");
        fontgen_log ("#if !defined (__%s_H__)\n"
                     "#define __%s_H__\n\n"
                     , to_upper (font_name), to_upper (font_name));
        fontgen_log ("static unsigned char const %s_data [] %s = {\n",
                font_name, options.progmem ? "PROGMEM" : "");

        glyphs = (struct font_glyph*) malloc (sizeof (struct font_glyph) * sample_len);
    }


    for (i = 0; i < sample_len; ++i)
    {
        int h, hh;
        int space = 0;

        if (options.sample [i] == ' ')
            space = 1;

        if (!space)
            index = FT_Get_Char_Index (g_face, options.sample [i]);
        else
            index = FT_Get_Char_Index (g_face, 'X');

        CHECK_FT (FT_Load_Glyph (g_face, index, FT_LOAD_TARGET_MONO));
        CHECK_FT (FT_Get_Glyph (g_face->glyph, &glyph));

        CHECK_FT (FT_Glyph_To_Bitmap (&glyph, FT_RENDER_MODE_MONO, 0, 1));
        bm_glyph = (FT_BitmapGlyph)glyph;
        bitmap = &bm_glyph->bitmap;

        if (!options.display)
        {
            h = (bitmap->rows >> 3) + (bitmap->rows & 0x7 ? 1 : 0);

            glyphs [glyph_count].code = options.sample [i];
            glyphs [glyph_count].size = ((h << 6) | (bitmap->width & 0x3F));
            glyphs [glyph_count].data = font_bytes;
            ++glyph_count;

            total_bytes += 6;

            fontgen_log ("    ");

            if (options.rotate)
            {
                for (j = 0; j < bitmap->rows; ++j)
                {
                    int w = 0;
                    int wb = bitmap->width / 8 + (bitmap->width & 0x7 ? 1 : 0);
                    for (w = 0; w < wb; ++w)
                    {
                        unsigned char b = bitmap->buffer[j * bitmap->pitch + w];
                        if (space)
                            b = 0;
                        fontgen_log ("0x%x, ", b);
                        ++total_bytes;
                        ++font_bytes;
                    }
                }

            }
            else
            {
                for (hh = 0; hh < h; ++hh)
                {
                    int rows = hh < (h - 1) ? 8 : (bitmap->rows & 0x7);
                    for (k = 0; k < bitmap->width; ++k)
                    {
                        unsigned char b = 0;
                        for (j = 0; j < rows; ++j)
                        {
                            int idx = k & 0x07;
                            unsigned char buf = bitmap->buffer[(j + (hh << 3)) * bitmap->pitch + (k >> 3)];
                            b |= ((buf & (1 << (7 - idx))) >> (7 - idx)) << (j & 0x7);
                        }
                        if (space)
                            b = 0;
                        fontgen_log ("0x%x, ", b);
                        ++total_bytes;
                        ++font_bytes;
                    }
                }
            }
            space = 0;
            fontgen_log ("\n");
        }
        else
        {
            printf ("'%c': wh=%d:%d(%d)\n",
                    options.sample [i], bitmap->width, bitmap->rows,
                    (bitmap->rows >> 3) + (bitmap->rows & 0x7 ? 1 : 0));
            for (j = 0; j < bitmap->rows; ++j)
            {
                int w = 0;
                int wb = bitmap->width / 8 + (bitmap->width & 0x7 ? 1 : 0);
                for (w = 0; w < wb; ++w)
                {
                    int k = 8;
                    int b = bitmap->buffer[j * bitmap->pitch + w];
                    while (k--)
                    {
                        if ((b & 0x80) && !space)
                            printf ("*");
                        else
                            printf (" ");
                        b <<= 1;
                    }
                }
                printf ("\n");
            }
            printf ("\n");
        }

        FT_Done_Glyph (glyph);
    }

    if (!options.display)
    {
        fontgen_log ("};\n\n");

        /* output glyph array */
        fontgen_log ("#if !defined (FONT_DATA_STRUCT)\n"
                     "#define FONT_DATA_STRUCT\n\n"
                     "typedef struct font_data\n"
                     "{\n"
                     "    unsigned char code;\n"
                     "    unsigned char size;\n"
                     "    unsigned char const *data;\n"
                     "} font_data_t;\n\n"
                     "#endif\n\n"
                     "#define %s_NUM_GLYPS %d\n"
                     "static font_data_t const %s [] %s =\n"
                     "{\n",
                     to_upper (font_name), sample_len, font_name, options.progmem ? "PROGMEM" : "");
        for (i = 0; i < sample_len; ++i)
        {
            fontgen_log ("    { '%c', 0x%x, &%s_data [%d] },\n",
                    glyphs [i].code, glyphs [i].size, font_name, glyphs [i].data);
        }
        fontgen_log ("};\n");
        fontgen_log ("/* total bytes: %d */\n", total_bytes);
        fontgen_log ("#endif //__%s_H__\n", to_upper (font_name));
    }

    if (glyphs)
        free (glyphs);
    CHECK_FT (FT_Done_Face (g_face));
    CHECK_FT (FT_Done_FreeType (g_library));
    return 0;
}
