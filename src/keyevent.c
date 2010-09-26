
/*

cellwriter -- a character recognition input method
Copyright (C) 2007 Michael Levin <risujin@risujin.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "hildon-im-ui.h"

#include "common.h"
#include "keys.h"
#include <string.h>
#include <stdlib.h>
//#include <X11/X.h>
//#include <X11/Xlib.h>
//#include <X11/keysym.h>
//#include <X11/extensions/XTest.h>
//#include <gdk/gdkx.h>

/* Define this to always overwrite an unused KeyCode to send any KeySym */
/* #define ALWAYS_OVERWRITE */

/* Define this to print key events without actually generating them */
/* #define DEBUG_KEY_EVENTS */

/*
        X11 KeyCodes
*/

enum {
        KEY_TAKEN = 0,   /* Has KeySyms, cannot be overwritten */
        KEY_BAD,         /* Manually marked as unusable */
        KEY_USABLE,      /* Has no KeySyms, can be overwritten */
        KEY_ALLOCATED,   /* Normally usable, but currently allocated */
        /* Values greater than this represent multiple allocations */
};

extern HildonIMUI *ui;

static char usable[256], pressed[256];
static int key_min, key_max, key_offset, key_codes;
//static KeySym *keysyms = NULL;
//static XModifierKeymap *modmap = NULL;

/* Bad keycodes: Despite having no KeySym entries, certain KeyCodes will
   generate special KeySyms even if their KeySym entries have been overwritten.
   For instance, KeyCode 204 attempts to eject the CD-ROM even if there is no
   CD-ROM device present! KeyCode 229 will launch GNOME file search even if
   there is no search button on the physical keyboard. There is no programatic
   way around this but to keep a list of commonly used "bad" KeyCodes. */

void bad_keycodes_read(void)
{
  /*
        int keycode;

        while (!profile_sync_int(&keycode)) {
                if (keycode < key_min || keycode > key_max) {
                        g_warning("Cannot block bad keycode %d, out of range",
                                  keycode);
                        continue;
                }
                usable[keycode] = KEY_BAD;
        }
  */
}

void bad_keycodes_write(void)
{
  /*
        int i;

        profile_write("bad_keycodes");
        for (i = key_min; i < key_max; i++)
                if (usable[i] == KEY_BAD)
                        profile_write(va(" %d", i));
        profile_write("\n");
  */
}

static void press_keycode(KeyCode k)
/* Called from various places to generate a key-down event */
{
  //if (k >= key_min && k <= key_max)
  //              XTestFakeKeyEvent(GDK_DISPLAY(), k, True, 1);
}

static void release_keycode(KeyCode k)
/* Called from various places to generate a key-up event */
{
  //        if (k >= key_min && k <= key_max)
  //            XTestFakeKeyEvent(GDK_DISPLAY(), k, False, 1);
}

static void type_keycode(KeyCode k)
/* Key-down + key-up */
{
        press_keycode(k);
        release_keycode(k);
}

static void setup_usable(void)
/* Find unused slots in the key mapping */
{
  /*
        int i, found;

        /* Find all free keys 
        memset(usable, 0, sizeof (usable));
        for (i = key_min, found = 0; i <= key_max; i++) {
                int j;

                for (j = 0; j < key_codes &&
                     keysyms[(i - key_min) * key_codes + j] == NoSymbol; j++);
                if (j < key_codes) {
                        usable[i] = KEY_TAKEN;
                        continue;
                }
                usable[i] = KEY_USABLE;
                found++;
        }
        key_offset = 0;

        /* If we haven't found a usable key, it's probably because we have
           already ran once ad used them all up without setting them back 
        if (!found) {
                usable[key_max - key_min - 1] = KEY_USABLE;
                g_warning("Found no usable KeyCodes, restart the X server!");
        } else
                g_debug("Found %d usable KeyCodes", found);
  */
}

static void cleanup_usable(void)
/* Clear all the usable KeyCodes or we won't have any when we run again! */
{
  /*
        int i, bad, unused = 0, freed;

        for (i = 0, freed = 0, bad = 0; i <= key_max; i++)
                if (usable[i] >= KEY_USABLE) {
                        int j, kc_used = FALSE;

                        for (j = 0; j < key_codes; j++) {
                                int index = (i - key_min) * key_codes + j;

                                if (keysyms[index] != NoSymbol)
                                        kc_used = TRUE;
                                keysyms[index] = NoSymbol;
                        }
                        if (kc_used)
                                freed++;
                        else
                                unused++;
                } else if (usable[i] == KEY_BAD)
                        bad++;
        if (freed) {
                XChangeKeyboardMapping(GDK_DISPLAY(), key_min, key_codes,
                                       keysyms, key_max - key_min + 1);
                XFlush(GDK_DISPLAY());
        }
        g_debug("Free'd %d KeyCode(s), %d unused, %d marked bad",
                freed, unused, bad);
  */
}

static void release_held_keys(void)
/* Release all held keys that were not pressed by us */
{
  /*
        int i;
        char keys[32];

        XQueryKeymap(GDK_DISPLAY(), keys);
        for (i = 0; i < 32; i++) {
                int j;

                for (j = 0; j < 8; j++) {
                        KeyCode keycode;

                        keycode = i * 8 + j;
                        if (keys[i] & (1 << j) && !pressed[keycode]) {
                                g_debug("Released held KeyCode %d", keycode);
                                release_keycode(keycode);
                        }
                }
        }
  */
}

/*
        Key Events
*/

int key_overwrites = 0, key_recycles = 0,
    key_shifted = 0, key_num_locked = FALSE, key_caps_locked = FALSE,
    key_disable_overwrite = FALSE;

static int alt_mask, num_lock_mask, meta_mask;
static KeyEvent ke_shift, ke_enter, ke_num_lock, ke_caps_lock;

static void reset_keyboard(void)
/* In order to reliably predict key event behavior we need to be able to
   reset the keyboard modifier and pressed state */
{
  /*
  Window root, child;
        int root_x, root_y, win_x, win_y;
        unsigned int mask;

        release_held_keys();
        XQueryPointer(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()),
                      &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
        if (mask & LockMask)
                type_keycode(ke_caps_lock.keycode);
        if (mask & num_lock_mask)
                type_keycode(ke_num_lock.keycode);
  */
}
/*
static int is_modifier(unsigned int keysym)
/* Returns TRUE for KeySyms that are tracked internally 
{
        switch (keysym) {
        case XK_Shift_L:
        case XK_Shift_R:
        case XK_Num_Lock:
        case XK_Caps_Lock:
                return TRUE;
        default:
                return FALSE;
        }
}*/

static void key_event_allocate(KeyEvent *key_event, unsigned int keysym)
/* Either finds the KeyCode associated with the given keysym or overwrites
   a usable one to generate it */
{
  /*
        int i, start;

        //* Invalid KeySym 
        if (!keysym) {
                key_event->keycode = -1;
                key_event->keysym = 0;
                return;
        }

        /* First see if our KeySym is already in the mapping 
        key_event->shift = FALSE;
#ifndef ALWAYS_OVERWRITE
        for (i = 0; i <= key_max - key_min; i++) {
                if (keysyms[i * key_codes + 1] == keysym)
                        key_event->shift = TRUE;
                if (keysyms[i * key_codes] == keysym || key_event->shift) {
                        key_event->keycode = key_min + i;
                        key_recycles++;

                        /* Bump the allocation count if this is an
                           allocateable KeyCode 
                        if (usable[key_event->keycode] >= KEY_USABLE)
                                usable[key_event->keycode]++;

                        return;
                }
        }
#endif

        /* Key overwrites may be disabled, in which case we're out of luck 
        if (key_disable_overwrite) {
                key_event->keycode = -1;
                key_event->keysym = 0;
                g_warning("Not allowed to overwrite KeyCode for %s",
                          XKeysymToString(keysym));
                return;
        }

        /* If not, find a usable KeyCode in the mapping 
        for (start = key_offset++; ; key_offset++) {
                if (key_offset > key_max - key_min)
                        key_offset = 0;
                if (usable[key_min + key_offset] == KEY_USABLE &&
                    !pressed[key_min + key_offset])
                        break;

                /* If we can't find one, invalidate the event 
                if (key_offset == start) {
                        key_event->keycode = -1;
                        key_event->keysym = 0;
                        g_warning("Failed to allocate KeyCode for %s",
                                  XKeysymToString(keysym));
                        return;
                }
        }
        key_overwrites++;
        key_event->keycode = key_min + key_offset;
        usable[key_event->keycode] = KEY_ALLOCATED;

        /* Modify the slot to hold our character 
        keysyms[key_offset * key_codes] = keysym;
        keysyms[key_offset * key_codes + 1] = keysym;
        XChangeKeyboardMapping(GDK_DISPLAY(), key_event->keycode, key_codes,
                               keysyms + key_offset * key_codes, 1);
        XSync(GDK_DISPLAY(), False);

        g_debug("Overwrote KeyCode %d for %s", key_event->keycode,
                XKeysymToString(keysym));
  */
}

void key_event_new(KeyEvent *key_event, unsigned int keysym)
/* Filters locks and shifts but allocates other keys normally */
{
        key_event->keysym = keysym;
        /*
        if (is_modifier(keysym))
                return;
        key_event_allocate(key_event, keysym);
        */
}

void key_event_free(KeyEvent *key_event)
/* Release resources associated with and invalidate a key event */
{
  /*
        if (key_event->keycode >= key_min && key_event->keycode <= key_max &&
            usable[key_event->keycode] == KEY_ALLOCATED)
                usable[key_event->keycode] = KEY_USABLE;
        key_event->keycode = -1;
        key_event->keysym = 0;
  */
}

void key_event_press(KeyEvent *key_event)
/* Press the KeyCode specified in the event */
{
  /*
        /* Track modifiers without actually using them 
        if (key_event->keysym == XK_Shift_L ||
            key_event->keysym == XK_Shift_R) {
                key_shifted++;
                return;
        } else if (key_event->keysym == XK_Caps_Lock) {
                key_caps_locked = !key_caps_locked;
                return;
        } else if (key_event->keysym == XK_Num_Lock) {
                key_num_locked = !key_num_locked;
                return;
        }

        /* Invalid event 
        if (key_event->keycode < key_min || key_event->keycode > key_max)
                return;

        /* If this KeyCode is already pressed, something is wrong 
        if (pressed[key_event->keycode]) {
                g_debug("KeyCode %d is already pressed", key_event->keycode);
                return;
        }

        /* Keep track of what KeyCodes we pressed down 
        pressed[key_event->keycode] = TRUE;

        /* Press our keycode 
        if (key_event->shift)
                press_keycode(ke_shift.keycode);
        press_keycode(key_event->keycode);
        XSync(GDK_DISPLAY(), False);
  */
}

void key_event_release(KeyEvent *key_event)
{
  /*
        /* Track modifiers without actually using them 
        if (key_event->keysym == XK_Shift_L ||
            key_event->keysym == XK_Shift_R) {
                key_shifted--;
                return;
        }

        /* Invalid key event 
        if (key_event->keycode < key_min || key_event->keycode > key_max)
                return;

        /* Keep track of what KeyCodes are pressed because of us 
        pressed[key_event->keycode] = FALSE;

        /* Release our keycode 
        release_keycode(key_event->keycode);
        if (key_event->shift)
                release_keycode(ke_shift.keycode);
        XSync(GDK_DISPLAY(), False);
  */
}

void unicode_to_utf8(unsigned int code, char *out){
  // little endian
  unsigned char *c = (unsigned char*)&code;
  if(code < 0x80){
    out[0] = c[0];
    out[1] = 0;
  }else if(code < 0x800){
    out[0] = 0xC0 | (c[1] << 2) | (c[0] >> 6);
    out[1] = 0x80 | (0x3F & c[0]);
    out[2] = 0;
  }else *out = 0; // todo
}

void key_event_send_char(int unichar)
{
  char string[6];

  unicode_to_utf8((unsigned int)unichar, string);
  
  if(ui){
    if(unichar == '\n')
      hildon_im_ui_send_communication_message(ui, HILDON_IM_CONTEXT_HANDLE_ENTER);
    else
      hildon_im_ui_send_utf8(ui, string);
  }

  /*
        KeyEvent key_event;
        KeySym keysym;

        /* Get the keysym for the unichar (may be unsupported) 
        keysym = XStringToKeysym(va("U%04X", unichar));
        if (!keysym) {
                g_warning("XStringToKeysym failed to get Keysym for '%C'",
                          unichar);
                return;
        }

        key_event_new(&key_event, keysym);
        key_event_press(&key_event);
        key_event_release(&key_event);
        key_event_free(&key_event);
        */
}

void key_event_send_enter()
{
  //type_keycode(ke_enter.keycode);
}

int key_event_init()
{

        /* Clear the array that keeps track of our pressed keys */
        memset(pressed, 0, sizeof (pressed));

        return 0;
}

void key_event_cleanup(void)
{
}
