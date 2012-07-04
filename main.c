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
 *         Author:  Valery Volgutov (Valery Volgutov), valery.volgutov@lge.com
 *        Company:  LGE
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
    printf ("\n");

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


int main (int argc, char *argv[])
{
    int font_size = 8;
    char const *font_name = NULL;
    char const *font_sample = NULL;
    printf ("fontgen\n");

    if (argc < 4)
    {
        printf ("usage: fontgen <font path> <size> <sample>\n");
        return 1;
    }

    font_name = argv[1];
    font_size = atoi (argv[2]);
    font_sample = argv[3];

    CHECK_FT (FT_Init_FreeType (&g_library));
    CHECK_FT (FT_New_Face (g_library, font_name, 0, &g_face));

    CHECK_FT (FT_Set_Char_Size (g_face, 0, font_size << 6, 300, 300));
    CHECK_FT (FT_Set_Pixel_Sizes (g_face, 0, font_size));

    fontgen_log ("unsigned char font_%s_%d = {",
            g_face->family_name, font_size);
    {
        int i, j, k;
        FT_Bitmap *bitmap = NULL;
        FT_BitmapGlyph bm_glyph;
        FT_Glyph glyph;
        FT_Int index;

        for (i = 0; i < strlen (font_sample); ++i)
        {
            int h, hh;
            index = FT_Get_Char_Index (g_face, font_sample[i]);
            CHECK_FT (FT_Load_Glyph (g_face, index, FT_LOAD_TARGET_MONO));
            CHECK_FT (FT_Get_Glyph (g_face->glyph, &glyph));

            CHECK_FT (FT_Glyph_To_Bitmap (&glyph, FT_RENDER_MODE_MONO, 0, 1));
            bm_glyph = (FT_BitmapGlyph)glyph;
            bitmap = &bm_glyph->bitmap;

            /*printf ("glyph (%c): %d, %d, %d, %d, %d\n"*/
                    /*, font_sample[i], bitmap->rows, bitmap->width*/
                    /*, bitmap->pitch, bitmap->num_grays*/
                    /*, bitmap->pixel_mode);*/

            h = (bitmap->rows >> 3) + (bitmap->rows & 0x7 ? 1 : 0);

            printf ("    '%c', 0x%x, ", font_sample[i], ((h << 5) | (bitmap->width & 0x1F)));

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
                    printf ("0x%x, ", b);
                }
            }
            printf ("\n");

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
                        if ((b & 0x80))
                            printf ("*");
                        else
                            printf (" ");
                        b <<= 1;
                    }
                }
                printf ("\n");
            }

            FT_Done_Glyph (glyph);
        }
    }
    fontgen_log ("};\n");

    CHECK_FT (FT_Done_Face (g_face));
    CHECK_FT (FT_Done_FreeType (g_library));
    return 0;
}
