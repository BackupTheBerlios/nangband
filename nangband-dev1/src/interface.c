/*
 * interface.c - User interface functions
 * Copyright (c) 2001 Andrew Sidwell (takkaria) & Antony Sidwell (ajps).
 * andy@sidwell.org.uk & antony@isparp.co.uk
 *
 * This file is dual-licenced under both the Angband & GPL licences. It
 * may be redistributed under the terms of the GPL (version 2 or any
 * later version), or under the terms of the traditional Angband Licence.
 *
 * Angband Licence
 * --------------- 
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * GPL Licence Notice
 * ------------------
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or any later
 * version.
 */
#include "angband.h"

/* -------------------------------------------------------- ajps, 19/10/01 ---
 * Clears a box of the screen.
 * ------------------------------------------------------------------------ */
void clear_box(int start_x, int start_y, int end_x, int end_y)
{
    int x, y;

    for (y = start_y; y < ( end_y + 1); y++)
    {
        for (x = start_x; x < (end_x + 1); x++)
          Term_putch(x, y, TERM_WHITE, ' ');
    }

    return;
}

/* For the vertical menu funtion */
#define SCROLL_ALLOWANCE   3

/* ---------------------------------------------------- takkaria, 20/10/01 ---
 * The 'display surround box' funtion.
 *
 * This funtion displays a surround box starting from a given point, up to
 * it's width and height. Designed as a companion to display_vertical_menu.
 * You can specify the char to print the box with as well as the colour.
 * ------------------------------------------------------------------------ */
void show_surround_box(int x, int y, char surround_char, int width, int height, int terminal_colour)
{
    int n;              /* Looper */
    
    /* First of all, we print the top of the box */
    for (n = 1; n < width; n++) Term_putch(x+n, y, terminal_colour, surround_char);
    
    /* Now we print the bit in the middle */
    for (n = 0; n < height; n++, y++)
    {
        Term_putch(x, y, terminal_colour, surround_char);
        Term_putch(x + width, y, terminal_colour, surround_char);
    }
    
    y--;
    
    /* Print the final line of surround_chars */
    for (n = 0; n < width; n++) Term_putch(x+n, y, terminal_colour, surround_char);

    return;
}

/* ------------------------------------------------------ ajps22, xx/10/01 ---
 * The 'vertical menu' funtion.
 *
 * Displays a vertical menu at a given point on the screen, with a specified
 * menu length and number of entries, plus an option to add letters in the
 * form of 'a)' or 'z)' before the options.
 * ------------------------------------------------------------------------ */
int show_vertical_menu(int x_pos, int y_pos, s16b menu_length, s16b current_choice, s16b no_choices, char *entries[],s16b *offset, bool add_letters)
{
    s16b n;
    s16b location = 0;
    char buffer[4]="";
    
    /*
     * If there are fewer choices than lines, don't try to go past the
     * number of choices
     */
    if ( no_choices < menu_length ) menu_length = no_choices;
                                          
    /* Store the line the current choice is on */
    location = ( current_choice - *offset );

    /*
     * If this is smaller than 'x', adjust the offset
     * to make it just on the top of the menu.
     */
    if ( location < SCROLL_ALLOWANCE )
        *offset = current_choice - SCROLL_ALLOWANCE;
           
    /*
     * If this is larger than 'x', adjust the offset
     * to make it just on the bottom of the menu.
     */
    if ( location > (menu_length - SCROLL_ALLOWANCE - 1) ) 
    *offset = current_choice - menu_length + SCROLL_ALLOWANCE + 1;

    /* If offset is out of bounds, set to the min/max value allowed */
    if ( *offset < 0 ) *offset = 0;
    if ( *offset > (no_choices - menu_length)) *offset = no_choices - menu_length;

    /* Print the menu to the screen */
    for (n = 0; n < menu_length; n++)
    {  
        if ( add_letters )
        {
            
            sprintf(buffer,"%c) ",I2A(n+*offset));
            
            Term_putstr(
               x_pos, y_pos+n, -1,
               ((n+*offset) != current_choice) ? TERM_WHITE : TERM_L_BLUE,
               buffer);
           
        }
            
        Term_putstr(
           add_letters ? x_pos+3 : x_pos, y_pos+n, -1,
           ((n+*offset) != current_choice) ? TERM_WHITE : TERM_L_BLUE,
           entries[n+*offset]);
    }

    Term_gotoxy(x_pos, y_pos + current_choice);

    /*
     * Return the new offset - we need this next time this fn is called.
     * Just done for fun - it has already been altered anyway.
     */

    return(*offset);
}

/* ---------------------------------------------------- takkaria, 20/12/01 ---
 * A new, Hengband-like 'window' function.
 *
 * Should be quite fun to use. :)
 * ------------------------------------------------------------------------ */
void Window_Make(int origin_x, int origin_y, int end_x, int end_y)
{
    int n;

    clear_box(origin_x - 1, origin_y - 1, end_x + 1, end_y + 1);

    Term_putch(origin_x, origin_y, TERM_WHITE, '+');
    Term_putch(end_x,    origin_y, TERM_WHITE, '+');
    Term_putch(origin_x, end_y,    TERM_WHITE, '+');
    Term_putch(end_x,    end_y,    TERM_WHITE, '+');

    for (n = 1; n < (end_x - origin_x); n++)
    {
        Term_putch(origin_x + n, origin_y, TERM_WHITE, '-');
        Term_putch(origin_x + n, end_y,    TERM_WHITE, '-');
    }

    for (n = 1; n < (end_y - origin_y); n++)
    {
        Term_putch(origin_x, origin_y + n, TERM_WHITE, '|');
        Term_putch(end_x,    origin_y + n, TERM_WHITE, '|');
    }
}

#define SCREEN_MIDDLE_X (Term->wid / 2)
#define SCREEN_MIDDLE_Y (Term->hgt / 2)
#define PROMPT_CENTRE ((strlen(question) / window_width) + ((strlen(question) % window_width) == 0 ? 0 : 1) + 5)

/* ---------------------------------------------------- takkaria, dd/mm/yr ---
 * Make a window, (window_width) long, and filled with (question).
 *
 * The macros above are used for simplicity.
 * ------------------------------------------------------------------------ */
bool Window_Prompt(int window_width, char *question)
{
    int x1, y1, x2, y2;
    int a1, b1, a2, b2;
    char ch;
    bool boolean = TRUE;

    x1 = SCREEN_MIDDLE_X - (window_width / 2);
    y1 = SCREEN_MIDDLE_Y - (PROMPT_CENTRE / 2);
    x2 = SCREEN_MIDDLE_X + (window_width / 2);
    y2 = SCREEN_MIDDLE_Y + (PROMPT_CENTRE / 2);

    a1 = SCREEN_MIDDLE_X - (strlen("Yes") + 1);
    b1 = y2 - 2;
    a2 = SCREEN_MIDDLE_X + (strlen("No") + 1);
    b2 = b1;

    Window_Make(x1, y1, x2, y2);

    put_text_block(x1 + 2, y1 + 1, window_width - 2, question, TERM_L_BLUE, FALSE, ALIGN_CENTRE);

    Term_putstr(a1 - 2, b1, -1, TERM_WHITE, ">"); 

    Term_putstr(a1, b1, -1, (boolean ? TERM_L_BLUE : TERM_WHITE), "Yes");
    Term_putstr(a2, b2, -1, (boolean ? TERM_WHITE : TERM_L_BLUE), "No");

    Term_gotoxy(a1 - 2, b1);

    while (TRUE)
    {
        ch = inkey();

        if (ch == '6' || ch == '4') boolean = !boolean;
        else if (FORCELOWER(ch) == 'n') return(FALSE);
        else if (FORCELOWER(ch) == 'y') return(TRUE);
        else if (ch == '\r' || ch == '\n') return(boolean);

        Term_putstr(a1, b1, -1, (boolean ? TERM_L_BLUE : TERM_WHITE), "Yes");
        Term_putstr(a2, b2, -1, (boolean ? TERM_WHITE : TERM_L_BLUE), "No");

        Term_putstr(a1 - 2, b1, -1, TERM_L_BLUE, (boolean ? ">" : " "));
        Term_putstr(a2 - 2, b2, -1, TERM_L_BLUE, (boolean ? " " : ">"));

        Term_gotoxy((boolean ? a1 - 2 : a2 - 2), b1);
    }

}
