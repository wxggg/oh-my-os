#pragma once

int usr_fs_init(void);
int usr_debug_init(void);
int usr_thread_init(void);
int mem_init(void);

int start_new_shell(void);
int usr_init(void);

extern struct thread *shell;
