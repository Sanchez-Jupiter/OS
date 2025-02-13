/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "elf.h"

#include "spike_interface/spike_utils.h"
extern elf_ctx Elfloader;
//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  sprint(buf);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process). 
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}

ssize_t sys_user_backtrace(int64 depth) {
  uint64 user_sp = current->trapframe->regs.sp + 16 + 8 + 16;
  int64 d = 0;
  for (uint64 p = user_sp; d < depth && (*(uint64*)p); d++, p += 16) {
    int index = -1;
    uint64 funcName = 0;                               // 最接近的函数的函数名
    for (int i = 0; i < Elfloader.syms_count; i++) {
      if (Elfloader.syms[i].st_info == STT_FUNC        // 是函数
        && Elfloader.syms[i].st_value < (*(uint64*)p)  // 入口地址在ra之前
        && Elfloader.syms[i].st_value > funcName) {    // 确保最接近
          funcName = Elfloader.syms[i].st_value;
          index = i;
      }
    }
    if (index + 1) {
      sprint("%s\n", &Elfloader.strtb[Elfloader.syms[index].st_name]);
    }
  }
  return 0;
}


//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    case SYS_user_backtrace:
      return sys_user_backtrace(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
