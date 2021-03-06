// Tests free_checks_tail_magic=1.
// RUN: %clang_hwasan  %s -o %t
// RUN: %env_hwasan_opts=free_checks_tail_magic=0     %run %t
// RUN: %env_hwasan_opts=free_checks_tail_magic=1 not %run %t 2>&1 | FileCheck %s
// RUN:                                           not %run %t 2>&1 | FileCheck %s

// REQUIRES: stable-runtime

#include <stdlib.h>
#include <stdio.h>
#include <sanitizer/hwasan_interface.h>

#include "utils.h"

static volatile char *sink;

// Overwrite the tail in a non-hwasan function so that we don't detect the
// stores as OOB.
__attribute__((no_sanitize("hwaddress"))) void overwrite_tail() {
  (*UNTAG(&sink))[20] = 0x42;
  (*UNTAG(&sink))[24] = 0x66;
}

int main(int argc, char **argv) {
  __hwasan_enable_allocator_tagging();

  char *p = (char*)malloc(20);
  sink = UNTAG(p);
  overwrite_tail();
  free(p);
// CHECK: ERROR: HWAddressSanitizer: alocation-tail-overwritten; heap object [{{.*}}) of size 20
// CHECK: in main {{.*}}tail-magic.c:[[@LINE-2]]
// CHECK: allocated here:
// CHECK: in main {{.*}}tail-magic.c:[[@LINE-7]]
// CHECK: Tail contains: .. .. .. .. 42 {{.. .. ..}} 66
}
