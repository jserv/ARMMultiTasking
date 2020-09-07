#include "common/trace.h"
#include "user/thread.h"
#include "user/util.h"
#include "common/assert.h"

extern void work_finished(void);

static void set_thread_pc(int tid, void* pc) {
  RegisterContext ctx;
  get_thread_registers(tid, (RegisterContext*)&ctx);
  // Remove bottom bit for thumb
  ctx.pc = ((size_t)pc) & ~1;
  set_thread_registers(tid, ctx);
}

void jump_here() {
  log_event("Changed registers!");
  // No point returning because our LR will be bogus
  thread_wait();
}

void pre_jumped() {
  // Will be redirected by setup
  __builtin_unreachable();
}

void jump_self() {
  // This proves that you can modify your own registers
  // since the thread is saved before every syscall.
  set_thread_pc(CURRENT_THREAD, jump_here);
  __builtin_unreachable();
}

void tracee() {
  log_event("Working...");
  yield();
  work_finished();
}

// Note: the original plan was to write this svc in manually
// However this code will be in ROM so that couldn't be done.

__attribute__((naked, used)) void __work_finished(void) {
  // By defining work_finished in assembly we can be sure of it's address
  asm volatile(".global work_finished\n\t"
               "work_finished:\n\t"
               "svc %0\n\t"
               // If the tracer doesn't redirect us we'll loop forever
               "b tracee\n\t"
               :
               : "i"(svc_thread_switch));
}

void finish_program(void) {
  log_event("Finished program");
  exit(0);
}

void tracer() {
  int tid = add_named_thread(tracee, "tracee");
  set_child(tid);

  RegisterContext ctx;

  size_t target_pc = (size_t)work_finished;
#ifdef __thumb__
  // +2 for next instr
  target_pc += 2;
  // Stored PC doesn't include mode bit so remove it
  target_pc &= ~1;
#else
  target_pc += 4;
#endif

  while (ctx.pc != target_pc) {
    yield();
    get_thread_registers(tid, &ctx);
  }
  log_event("Hit work_finished!");

  // Now redirect to prevent infinite loop
  ctx.pc = (size_t)finish_program;
  assert(set_thread_registers(tid, ctx));
  log_event("Redirected tracee");
  yield();
}

void setup(void) {
  set_thread_name(CURRENT_THREAD, "setup");

  // Has registers changed even before running
  int tid = add_named_thread(pre_jumped, "prejumped");
  set_thread_pc(tid, jump_here);
  yield();

  // Changes its own registers
  add_named_thread(jump_self, "jumpself");
  yield();
  add_named_thread(tracer, "tracer");
}
