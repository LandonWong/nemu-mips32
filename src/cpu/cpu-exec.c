#include <sys/time.h>

#include "nemu.h"
#include "cpu/exec.h"
#include "monitor/monitor.h"


#define EXCEPTION_VECTOR_LOCATION 0x10000020
#define MAX_INSTR_TO_PRINT 10

int nemu_state = NEMU_STOP;
extern int print_commit_log;

char asm_buf[80];
char *asm_buf_p;


static int dsprintf(char *buf, const char *fmt, ...) {
  if(!print_commit_log) return 0;
#if 0
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
#endif
  return 0;
}


void print_registers() {
  // printf("$base:     0x%08x\n", cpu.base);
  printf("$pc:    0x%08x    $hi:    0x%08x    $lo:    0x%08x\n", cpu.pc, cpu.hi, cpu.lo);
  printf("$0 :0x%08x  $at:0x%08x  $v0:0x%08x  $v1:0x%08x\n", cpu.gpr[0], cpu.gpr[1], cpu.gpr[2], cpu.gpr[3]);
  printf("$a0:0x%08x  $a1:0x%08x  $a2:0x%08x  $a3:0x%08x\n", cpu.gpr[4], cpu.gpr[5], cpu.gpr[6], cpu.gpr[7]);
  printf("$t0:0x%08x  $t1:0x%08x  $t2:0x%08x  $t3:0x%08x\n", cpu.gpr[8], cpu.gpr[9], cpu.gpr[10], cpu.gpr[11]);
  printf("$t4:0x%08x  $t5:0x%08x  $t6:0x%08x  $t7:0x%08x\n", cpu.gpr[12], cpu.gpr[13], cpu.gpr[14], cpu.gpr[15]);
  printf("$s0:0x%08x  $s1:0x%08x  $s2:0x%08x  $s3:0x%08x\n", cpu.gpr[16], cpu.gpr[17], cpu.gpr[18], cpu.gpr[19]);
  printf("$s4:0x%08x  $s5:0x%08x  $s6:0x%08x  $s7:0x%08x\n", cpu.gpr[20], cpu.gpr[21], cpu.gpr[22], cpu.gpr[23]);
  printf("$t8:0x%08x  $t9:0x%08x  $k0:0x%08x  $k1:0x%08x\n", cpu.gpr[24], cpu.gpr[25], cpu.gpr[26], cpu.gpr[27]);
  printf("$gp:0x%08x  $sp:0x%08x  $fp:0x%08x  $ra:0x%08x\n", cpu.gpr[28], cpu.gpr[29], cpu.gpr[30], cpu.gpr[31]);
}


int init_cpu() {
  assert(sizeof(cp0_status_t) == sizeof(cpu.cp0[CP0_STATUS][0]));
  assert(sizeof(cp0_cause_t) == sizeof(cpu.cp0[CP0_CAUSE][0]));
  cpu.gpr[29] = 0x18000000;
  cpu.cp0[CP0_STATUS][0] = 0x1000FF00;
  return 0;
}

static inline void trigger_exception(int code) {
  cpu.cp0[CP0_EPC][0] = cpu.pc;
  cpu.pc = EXCEPTION_VECTOR_LOCATION;

  cpu.base = 0; // kernel base is zero

  cp0_status_t *status = (void *)&(cpu.cp0[CP0_STATUS][0]);
  status->EXL = 1;
  status->IE = 0;

  cp0_cause_t *cause = (void *)&(cpu.cp0[CP0_CAUSE][0]);
  cause->ExcCode = code;
}


void check_interrupt() {
  if(cpu.cp0[CP0_COMPARE][0] == cpu.cp0[CP0_COUNT][0]) {
	trigger_exception(EXC_INTR);
  }
}

void update_cp0_timer() {
  cpu.cp0[CP0_COUNT][0] += 5; // add 5 cycles
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  for (; n > 0; n --) {
	// update_cp0_timer();
	
    asm_buf_p = asm_buf;
    asm_buf_p += dsprintf(asm_buf_p, "%8x:    ", cpu.pc);

    Inst inst = { .val = instr_fetch(&cpu.pc, 4) };

    asm_buf_p += dsprintf(asm_buf_p, "%08x    ", inst.val);

#include "exec-handlers.h"

    if(print_commit_log) print_registers();

#ifdef INTR
    check_interrupt();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
