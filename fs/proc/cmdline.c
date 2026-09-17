// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <asm/setup.h>

#if defined(CONFIG_INITRAMFS_IGNORE_SKIP_FLAG) || \
	defined(CONFIG_PROC_CMDLINE_APPEND_ANDROID_FORCE_NORMAL_BOOT)
/*
 * Both strings are 13 characters long on purpose: only the leading
 * "skip_initramf" part is rewritten, so the trailing 's' of the original
 * "skip_initramfs" token survives and the result reads "want_initramfs".
 *
 * Note that this only changes what /proc/cmdline reports to userspace.
 * Forcing the built-in initramfs to be used is decided in init/initramfs.c
 * (see CONFIG_INITRAMFS_IGNORE_SKIP_FLAG) and does not depend on this.
 */
#define INITRAMFS_STR_FIND "skip_initramf"
#endif

#ifdef CONFIG_INITRAMFS_IGNORE_SKIP_FLAG
#define INITRAMFS_STR_REPLACE "want_initramf"
#define INITRAMFS_STR_LEN (sizeof(INITRAMFS_STR_FIND) - 1)
#endif

#ifdef CONFIG_PROC_CMDLINE_APPEND_ANDROID_FORCE_NORMAL_BOOT
#define ANDROID_FORCE_NORMAL_BOOT_STR "androidboot.force_normal_boot=1"
#endif

static char proc_command_line[COMMAND_LINE_SIZE];

static void proc_command_line_init(void)
{
	strscpy(proc_command_line, saved_command_line, sizeof(proc_command_line));

#ifdef CONFIG_INITRAMFS_IGNORE_SKIP_FLAG
	{
		char *offset_addr;

		offset_addr = strstr(proc_command_line, INITRAMFS_STR_FIND);
		if (offset_addr)
			memcpy(offset_addr, INITRAMFS_STR_REPLACE,
			       INITRAMFS_STR_LEN);
	}
#endif

#ifdef CONFIG_PROC_CMDLINE_APPEND_ANDROID_FORCE_NORMAL_BOOT
	{
		size_t len;

		/*
		 * Match against saved_command_line, not proc_command_line:
		 * the replacement above has already removed the token.
		 */
		if (strstr(saved_command_line, INITRAMFS_STR_FIND)) {
			len = strlen(proc_command_line);

			if (len + 1 + sizeof(ANDROID_FORCE_NORMAL_BOOT_STR) <=
			    sizeof(proc_command_line)) {
				/* space, then the token including its NUL */
				proc_command_line[len] = ' ';
				memcpy(proc_command_line + len + 1,
				       ANDROID_FORCE_NORMAL_BOOT_STR,
				       sizeof(ANDROID_FORCE_NORMAL_BOOT_STR));
			} else {
				pr_warn("proc: cmdline too long, "
					ANDROID_FORCE_NORMAL_BOOT_STR
					" not appended\n");
			}
		}
	}
#endif
}

static int cmdline_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, proc_command_line);
	seq_putc(m, '\n');
	return 0;
}

static int __init proc_cmdline_init(void)
{
	proc_command_line_init();

	proc_create_single("cmdline", 0, NULL, cmdline_proc_show);
	return 0;
}
fs_initcall(proc_cmdline_init);
