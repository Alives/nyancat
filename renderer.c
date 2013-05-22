#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/time.h>
#include "colors.h"
#include "frames.h"

/*
 * Formats the bottom line of display to show total accumulated running time.
 */
void insert_time(char * buf, int * len, int term, unsigned long start) {
  int i, time_str_len, padding;
  int odd = 0;
  char * plural = "";
  char * spacer = colors[term][15];
  char time_str[FRAME_COLUMNS];
  unsigned long seconds;

  seconds = get_seconds() - start;
  if (seconds != 1) plural = "s";
  time_str_len = sprintf(time_str, "You have nyaned for %lu second%s!",
                         seconds, plural);
  if (time_str_len % 2 != 0) odd = 1;
  if (time_str_len > FRAME_COLUMNS)
    time_str_len = sprintf(time_str, "You win!");
  padding = ((FRAME_COLUMNS - time_str_len) / 2);
  for (i = 0; i < padding; i++) *len += sprintf(buf + *len, "%s", spacer);
  // Set colors to bold white and append the time string.
  *len += sprintf(buf + *len, "%s%s%s", colors[term][14],
                  time_str, colors[term][0]);
  for (i = 0; i < (padding + odd); i++)
    *len += sprintf(buf + *len, "%s", spacer);
  // Reset colors.
  *len += sprintf(buf + *len, "\033[00m");
}

void render_all_frames(char ** bufs, int * lengths, int term) {
  int f, x, y, hex_val;
  char last;
  char * spacer = colors[term][15];
  char str[2];
  char * end;

  for (f = 0; f < TOTAL_FRAMES; f++) {
    last = 'G'; // Outside the 0-F hex range used in frames.h.
    // Reset the cursor position, reset the colors,
    // set buf_len to 0, and purge the buffer.
    lengths[f] = sprintf(bufs[f], "\033[00m\033[H");
    // Comes out to be about 25 rows.
    for (y = 20; y < 43; y++) {
      // Since blocks are double, this makes 80 columns.
      for (x = 10; x < 50; ++x) { // There is data for 64x64 grid though.
        sprintf(str, "%c", frames[f][y][x]);
        hex_val = simple_strtol(str, &end, 16);
        if (term == 2) { // vt220 doesn't support colors.
          lengths[f] += sprintf(bufs[f] + lengths[f], "%s", colors[term][hex_val]);
        } else {
          // Terminals that support color codes.
          if ((frames[f][y][x] != last) &&
              (strlen(colors[term][hex_val]) > 0)) {
            last = frames[f][y][x];
            lengths[f] += sprintf(bufs[f] + lengths[f], "%s%s%s",
                                  colors[term][hex_val], spacer, spacer);
          } else {
            // No color codes needed. The current region has the same color as
            // the previous one.
            lengths[f] += sprintf(bufs[f] + lengths[f], "%s%s", spacer, spacer);
          }
        }
      }
      // Next line of rendering.
      lengths[f] += sprintf(bufs[f] + lengths[f], "\n");
    }
    lengths[f] = strlen(bufs[f]); // Just to be safe.
  }
}
