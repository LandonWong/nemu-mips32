#include <SDL/SDL.h>
#include <stdbool.h>

#include "device.h"

#define SERIAL_QUEUE_LEN 1024
static int serial_queue[SERIAL_QUEUE_LEN];
static int serial_f = 0, serial_r = 0;

// wait to be completed
const char *SDLK_to_ascii[SDLK_LAST] = {
    [SDLK_ESCAPE] = "\e",
    [SDLK_F1] = "\eOP",
    [SDLK_F2] = "\eOQ",
    [SDLK_F3] = "\eOR",
    [SDLK_F4] = "\eOS",
    [SDLK_F5] = "\e[15~",
    [SDLK_F6] = "\e[17~",
    [SDLK_F7] = "\e[18~",
    [SDLK_F8] = "\e[19~",
    [SDLK_F9] = "\e[20~",
    [SDLK_F10] = "\e[21~",
    [SDLK_F11] = "\e[22~",
    [SDLK_F12] = "\e[24~",
    [SDLK_BACKQUOTE] = "`",
    [SDLK_1] = "1",
    [SDLK_2] = "2",
    [SDLK_3] = "3",
    [SDLK_4] = "4",
    [SDLK_5] = "5",
    [SDLK_6] = "6",
    [SDLK_7] = "7",
    [SDLK_8] = "8",
    [SDLK_9] = "9",
    [SDLK_0] = "0",
    [SDLK_MINUS] = "-",
    [SDLK_EQUALS] = "=",
    [SDLK_BACKSPACE] = "\x8",
    [SDLK_TAB] = "\t",
    [SDLK_q] = "q",
    [SDLK_w] = "w",
    [SDLK_e] = "e",
    [SDLK_r] = "r",
    [SDLK_t] = "t",
    [SDLK_y] = "y",
    [SDLK_u] = "u",
    [SDLK_i] = "i",
    [SDLK_o] = "o",
    [SDLK_p] = "p",
    [SDLK_LEFTBRACKET] = "{",
    [SDLK_RIGHTBRACKET] = "}",
    [SDLK_SLASH] = "\\",
    [SDLK_a] = "a",
    [SDLK_s] = "s",
    [SDLK_d] = "d",
    [SDLK_f] = "f",
    [SDLK_g] = "g",
    [SDLK_h] = "h",
    [SDLK_j] = "j",
    [SDLK_k] = "k",
    [SDLK_l] = "l",
    [SDLK_SEMICOLON] = ";",
    [SDLK_QUOTE] = "'",
    [SDLK_RETURN] = "\n",
    [SDLK_LSHIFT] = "", // cannot be escaped
    [SDLK_z] = "z",
    [SDLK_x] = "x",
    [SDLK_c] = "c",
    [SDLK_v] = "v",
    [SDLK_b] = "b",
    [SDLK_n] = "n",
    [SDLK_m] = "m",
    [SDLK_COMMA] = ",",
    [SDLK_PERIOD] = ".",
    [SDLK_BACKSLASH] = "/",
    [SDLK_RSHIFT] = "", // cannot be escaped
    [SDLK_LCTRL] = "",  // cannot be escaped
    [SDLK_LALT] = "",   // cannot be escaped
    [SDLK_SPACE] = " ",
    [SDLK_RALT] = "",  // cannot be escaped
    [SDLK_RCTRL] = "", // cannot be escaped
    [SDLK_INSERT] = "\e[2~",
    [SDLK_HOME] = "\e[1~",
    [SDLK_PAGEUP] = "\e[5~",
    [SDLK_DELETE] = "\x7f",
    [SDLK_END] = "\e[4~",
    [SDLK_PAGEDOWN] = "\e[6~",
    [SDLK_UP] = "\e[A",
    [SDLK_LEFT] = "\e[D",
    [SDLK_DOWN] = "\e[B",
    [SDLK_RIGHT] = "\e[C",
    [SDLK_KP_DIVIDE] = "/",
    [SDLK_KP_MULTIPLY] = "*",
    [SDLK_KP_MINUS] = "-",
    [SDLK_KP_PLUS] = "+",
    [SDLK_KP7] = "7",
    [SDLK_KP8] = "8",
    [SDLK_KP9] = "9",
    [SDLK_KP4] = "4",
    [SDLK_KP5] = "5",
    [SDLK_KP6] = "6",
    [SDLK_KP1] = "1",
    [SDLK_KP2] = "2",
    [SDLK_KP3] = "3",
    [SDLK_KP0] = "0",
    [SDLK_KP_EQUALS] = "=",
    [SDLK_KP_ENTER] = "\n",
    [SDLK_KP_PERIOD] = ".",
};

void serial_enqueue(int ch) {
  int next = (serial_r + 1) % SERIAL_QUEUE_LEN;
  if (next != serial_f) { // if not full
    serial_queue[serial_r] = ch;
    serial_r = next;
  }
}

void serial_enqueue_SDLKey(SDL_EventType type, SDLKey key) {
  if (key < 0 || key >= SDLK_LAST) return;

  static bool sdlk_state[SDLK_LAST];
  // bool shift = false;
  if (type == SDL_KEYDOWN) {
    if (sdlk_state[(int)key]) return;
    sdlk_state[(int)key] = true;
  }

  if (type == SDL_KEYUP) {
    sdlk_state[(int)key] = false;
    return;
  }

  const char *p = SDLK_to_ascii[key];
  while (p && *p) {
    int next = (serial_r + 1) % SERIAL_QUEUE_LEN;
    if (next != serial_f) { // if not full
      serial_queue[serial_r] = *p;
      serial_r = next;
    } else {
      break;
    }
    p++;
  }
}

int serial_dequeue() {
  int data = serial_queue[serial_f];
  if (serial_f != serial_r) serial_f = (serial_f + 1) % SERIAL_QUEUE_LEN;
  return data;
}

int serial_queue_top() { return serial_queue[serial_f]; }

bool serial_queue_is_full() {
  int next = (serial_r + 1) % SERIAL_QUEUE_LEN;
  return next != serial_f;
}

bool serial_queue_is_empty() { return serial_f == serial_r; }
