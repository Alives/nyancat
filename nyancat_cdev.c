#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include "colors.h"
#include "frames.h"
#include "renderer.h"

#define MODULE_NAME "nyancat_cdev"
#define MIN(a,b) ((a)<(b)?(a):(b))
#define TERM_NAME_LEN 16             // Length of terminal name.

static char ** rendered_frames; // Buffer for pre-rendered frames.
static int * rendered_frame_lengths; // Lengths of the pre-rendered frame buffers.
static int current_rendered_term = -1;
static struct proc_dir_entry * proc_dir;
static struct proc_dir_entry * proc_delay;
static struct proc_dir_entry * proc_term;
static struct semaphore delay_sema;
static struct semaphore render_sema;
static struct semaphore term_sema;

struct nyan_s {
  int frame, pos, buf_len;
  char * buf;
  unsigned long start;
};

struct nyan_proc {
  int term;
  char term_name[TERM_NAME_LEN];
  unsigned long delay;
};

struct nyan_proc * nyan_proc_data;


/*
 * Respond to a client side read syscall. This function renders a new buffer and
 * handles copying it to the user's buffer.
 */
static ssize_t nyancat_read(struct file *file, char __user *userbuf,
                            size_t size, loff_t *pos) {
  struct nyan_s * nyan = file->private_data;
  int total, sent;
  int term;
  unsigned long delay;

  if (down_interruptible(&term_sema) != 0) { return -EINTR; }
  term = nyan_proc_data->term;
  up(&term_sema);

  if (down_interruptible(&render_sema) != 0) { return -EINTR; }
  strcpy(nyan->buf, rendered_frames[nyan->frame]);
  nyan->buf_len = rendered_frame_lengths[nyan->frame];
  up(&render_sema);

  insert_time(nyan->buf, &nyan->buf_len, term, nyan->start);

  total = MIN(size, nyan->buf_len);
  sent = total - copy_to_user(userbuf, nyan->buf + nyan->pos, total);
  if (sent + nyan->pos >= nyan->buf_len) {
    if (nyan->frame == TOTAL_FRAMES - 1) nyan->frame = 0; // 11 Frames in total.
    else nyan->frame++;
    nyan->pos = 0;
    nyan->buf_len = 0;

    if (down_interruptible(&delay_sema) != 0) { return -EINTR; }
    delay = nyan_proc_data->delay;
    up(&delay_sema);
    msleep_interruptible(delay); // Delay for proper animation.

  } else {
    nyan->pos += sent;
  }
  return sent;
}

/*
 * Initialize everything for the new session due an open syscall from the user.
 */
static int nyancat_open(struct inode *inode, struct file *file) {
  struct nyan_s * nyan = kmalloc(sizeof(struct nyan_s), GFP_KERNEL);
  nyan->pos = 0;
  nyan->frame = 0;
  nyan->buf_len = 0;
  nyan->buf = kmalloc(FRAME_BUFSIZE, GFP_KERNEL);
  nyan->start = get_seconds();
  file->private_data = nyan;
  return 0;
}

/*
 * Free malloc'd memory and close the session.
 */
static int nyancat_close(struct inode *inode, struct file *file) {
  struct nyan_s * nyan = file->private_data;
  kfree(nyan->buf);
  kfree(nyan);
  return 0;
}

/*
 * Return the current delay value (in ms) to the user via proc.
 */
static int delay_read_proc(char* page, char** start, off_t off, int count,
                           int* eof, void* data) {
  char * p = page;

  if (down_interruptible(&delay_sema) != 0) { return -EINTR; }
  p += sprintf(page, "%lu\n", nyan_proc_data->delay);
  up(&delay_sema);

  return p - page;
}

/*
 * Read the new delay value (in ms) from the user via proc.
 */
static int delay_write_proc(struct file* file, const char* buffer,
                            unsigned long count, void* data) {
  int len;
  int delay_len = sizeof(nyan_proc_data->delay);
  char kbuf[delay_len];
  char * end;
  unsigned long delay;

  len = MIN(count, delay_len);
  if (copy_from_user(kbuf, buffer, len) != 0) return -EFAULT;
  delay = simple_strtoul(kbuf, &end, 10);

  if (down_interruptible(&delay_sema) != 0) { return -EINTR; }
  if (nyan_proc_data->delay != delay) {
    nyan_proc_data->delay = delay;
    printk(KERN_INFO "nyancat device has been configured with %lu ms delay.\n",
           delay);
  }
  up(&delay_sema);

  return len;
}

/*
 * Return the current terminal value to the user via proc.
 */
static int term_read_proc(char* page, char** start, off_t off, int count,
                          int* eof, void* data) {
  char * p = page;

  if (down_interruptible(&term_sema) != 0) { return -EINTR; }
  p += sprintf(page, "%s\n", nyan_proc_data->term_name);
  up(&term_sema);

  return p - page;
}

/*
 * Read the new terminal value from the user via proc.
 */
static int term_write_proc(struct file* file, const char* buffer,
                           unsigned long count, void* data) {
  int i, len, term;
  int match = 0;
  char kbuf[TERM_NAME_LEN];
  char term_name[TERM_NAME_LEN];

  len = MIN(TERM_NAME_LEN, count);
  if (copy_from_user(kbuf, buffer, len) != 0) return -EFAULT;
  if (kbuf[len - 1] == '\n') kbuf[len - 1] = '\0';

  // This just sanitizes the terminal type input.
  for (i = 0; i < TOTAL_TERMINAL_TYPES; i++) {
    if (strstr(colors[i][16], kbuf) != NULL) {
      term = i;
      strcpy(term_name, colors[i][16]);
      match = 1;
      break;
    }
  }
  if (match == 0) {
    strcpy(term_name, "default");
    term = 5;
  }

  if (term != nyan_proc_data->term) {
    if (down_interruptible(&term_sema) != 0) { return -EINTR; }
    nyan_proc_data->term = term;
    strcpy(nyan_proc_data->term_name, term_name);
    up(&term_sema);
    // Re-render frames for the new terminal type.
    if (down_interruptible(&render_sema) != 0) { return -EINTR; }
    render_all_frames(rendered_frames, rendered_frame_lengths, term);
    current_rendered_term = term;
    up(&render_sema);
    printk(KERN_INFO "nyancat device has been configured for %s terminals.\n",
           term_name);
  }

  return len;
}

static struct file_operations nyancat_fops = {
  .open = nyancat_open,
  .read = nyancat_read,
  .release = nyancat_close,
};

static struct miscdevice nyancat_dev = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "nyancat",
  .fops = &nyancat_fops
};

/*
 * Remove the procfs file/dir entries.
 */
static void remove_procfs(void) {
  if (proc_delay) remove_proc_entry("delay_in_ms", proc_dir);
  if (proc_term) remove_proc_entry("term_type", proc_dir);
  if (proc_dir) remove_proc_entry("nyancat", NULL);
}

/*
 * Setup the proc_fs files.
 */
static int initialize_procfs(void) {
  proc_dir = proc_mkdir("nyancat", NULL);
  if (!proc_dir) {
    printk(KERN_ALERT "Could not create directory /proc/nyancat.\n");
    return 1;
  }
  proc_term = create_proc_entry("term_type", (mode_t)0666, proc_dir);
  if (!proc_term) {
    printk(KERN_ALERT "Could not create /proc/nyancat/term_type entry!\n");
    remove_procfs();
    return 1;
  }
  proc_delay = create_proc_entry("delay_in_ms", (mode_t)0666, proc_dir);
  if (!proc_delay) {
    printk(KERN_ALERT "Could not create /proc/nyancat/delay_in_ms entry!\n");
    remove_procfs();
    return 1;
  }
  nyan_proc_data = kmalloc(sizeof(struct nyan_proc), GFP_KERNEL);
  strcpy(nyan_proc_data->term_name, "xterm");
  nyan_proc_data->term = 0; // xterm by default
  nyan_proc_data->delay = 90;
  proc_delay->read_proc = delay_read_proc;
  proc_delay->write_proc = delay_write_proc;
  proc_term->read_proc = term_read_proc;
  proc_term->write_proc = term_write_proc;
  printk(KERN_INFO "nyancat proc files are under /proc/nyancat/.\n");
  printk(KERN_INFO "nyancat device is configured for %s terminals.\n",
         nyan_proc_data->term_name);
  printk(KERN_INFO "nyancat device is configured with %lu ms delay.\n",
         nyan_proc_data->delay);
  return 0;
}

/*
 * Initialize the module upon insertion.
 */
static int nyancat_module_init(void) {
  int i;

  // Setup semaphores.
  sema_init(&delay_sema, 1);
  sema_init(&render_sema, 1);
  sema_init(&term_sema, 1);

  initialize_procfs(); // Setup the proc_fs files.

  // Pre-render the frames for the default terminal type.
  rendered_frames = kmalloc(TOTAL_FRAMES * sizeof(char *), GFP_KERNEL);
  for (i = 0; i < TOTAL_FRAMES; i++)
    rendered_frames[i] = kmalloc(FRAME_BUFSIZE * sizeof(char *), GFP_KERNEL);
  rendered_frame_lengths = kmalloc(TOTAL_FRAMES * sizeof(int), GFP_KERNEL);
  render_all_frames(rendered_frames, rendered_frame_lengths,
                    nyan_proc_data->term);
  current_rendered_term = nyan_proc_data->term;

  if (misc_register(&nyancat_dev) != 0) {
    return 1;
  }
  printk(KERN_INFO "nyancat device has been added to the system.\n");
  return 0;
}

/*
 * Remove the module and free malloc'd memory.
 */
static void nyancat_module_exit(void) {
  int i;
  if (misc_deregister(&nyancat_dev) == 0) {
    remove_procfs();
    kfree(nyan_proc_data);
    for (i = 0; i < TOTAL_FRAMES; i++)
      kfree(rendered_frames[i]);
    kfree(rendered_frames);
    kfree(rendered_frame_lengths);
    printk(KERN_INFO "nyancat device has been removed from the system.\n");
  }
}

module_init(nyancat_module_init);
module_exit(nyancat_module_exit);

/* MODULE_AUTHOR("Elliott Friedman <elliott.friedman@gmail.com>");
 * MODULE_DESCRIPTION("\"Nyancat cdev module");
 * MODULE_LICENSE("GPL");
 * MODULE_VERSION("1.0");
 */
