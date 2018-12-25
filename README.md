## testcase proj1
make tests/userprog/args-none.result  
make tests/userprog/args-single.result  
make tests/userprog/args-multiple.result  
make tests/userprog/args-many.result  
make tests/userprog/args-dbl-space.result  
make tests/userprog/sc-bad-sp.result  
make tests/userprog/sc-bad-arg.result  
make tests/userprog/sc-boundary.result  
make tests/userprog/sc-boundary-2.result  
make tests/userprog/halt.result  
make tests/userprog/exit.result  
make tests/userprog/exec-once.result  
make tests/userprog/exec-arg.result  
make tests/userprog/exec-multiple.result  
make tests/userprog/exec-missing.result  
make tests/userprog/exec-bad-ptr.result  
make tests/userprog/wait-simple.result  
make tests/userprog/wait-twice.result  
make tests/userprog/wait-killed.result  
make tests/userprog/wait-bad-pid.result  
make tests/userprog/multi-recurse.result  

## testcase proj2
make tests/userprog/create-normal.result
make tests/userprog/create-empty.result
make tests/userprog/create-null.result
make tests/userprog/create-bad-ptr.result
make tests/userprog/create-long.result
make tests/userprog/create-exists.result  
make tests/userprog/create-bound.result  
make tests/userprog/open-normal.result 
make tests/userprog/open-missing.result
make tests/userprog/open-boundary.result
make tests/userprog/open-empty.result
make tests/userprog/open-null.result
make tests/userprog/open-bad-ptr.result
make tests/userprog/open-twice.result
make tests/userprog/close-normal.result
make tests/userprog/close-twice.result
make tests/userprog/close-stdin.result
make tests/userprog/close-stdout.result
make tests/userprog/close-bad-fd.result
make tests/userprog/read-normal.result
make tests/userprog/read-bad-ptr.result
make tests/userprog/read-boundary.result
make tests/userprog/read-zero.result
make tests/userprog/read-stdout.result
make tests/userprog/read-bad-fd.result
make tests/userprog/write-normal.result
make tests/userprog/write-bad-ptr.result
make tests/userprog/write-boundary.result
make tests/userprog/write-zero.result
make tests/userprog/write-stdin.result
make tests/userprog/write-bad-fd.result
make tests/userprog/multi-child-fd.result
make tests/userprog/rox-simple.result
make tests/userprog/rox-child.result
make tests/userprog/rox-multichild.result
make tests/userprog/bad-read.result
make tests/userprog/bad-write.result
make tests/userprog/bad-read2.result
make tests/userprog/bad-write2.result
make tests/userprog/bad-jump.result
make tests/userprog/bad-jump2.result
make tests/userprog/no-vm/multi-oom.result
make tests/filesys/base/lg-create.result
make tests/filesys/base/lg-full.result
make tests/filesys/base/lg-random.result
make tests/filesys/base/lg-seq-block.result
make tests/filesys/base/lg-seq-random.result
make tests/filesys/base/sm-create.result
make tests/filesys/base/sm-full.result
make tests/filesys/base/sm-random.result
make tests/filesys/base/sm-seq-block.result
make tests/filesys/base/sm-seq-random.result
make tests/filesys/base/syn-read.result
make tests/filesys/base/syn-remove.result
make tests/filesys/base/syn-write.result

## testcase proj3
make tests/threads/alarm-single.result
make tests/threads/alarm-multiple.result
make tests/threads/alarm-simultaneous.result
make tests/threads/alarm-priority.result
make tests/threads/alarm-zero.result
make tests/threads/alarm-negative.result
make tests/threads/priority-change.result
make tests/threads/priority-change-2.result
make tests/threads/priority-fifo.result
pintos -v -k -T 60 --qemu -- -q run priority-lifo
make tests/threads/priority-preempt.result
make tests/threads/priority-sema.result
make tests/threads/priority-aging.result

make tests/threads/mlfqs-block.result
make tests/threads/mlfqs-fair-2.result
make tests/threads/mlfqs-fair-20.result
make tests/threads/mlfqs-load-1.result
make tests/threads/mlfqs-load-60.result
make tests/threads/mlfqs-load-avg.result
make tests/threads/mlfqs-nice-10.result
make tests/threads/mlfqs-nice-2.result
make tests/threads/mlfqs-recent-1.result


## testcase proj4
make tests/vm/pt-grow-stack.result  
make tests/vm/pt-grow-pusha.result  
make tests/vm/pt-grow-bad.result  
make tests/vm/pt-big-stk-obj.result  
make tests/vm/pt-bad-addr.result  
make tests/vm/pt-bad-read.result  
make tests/vm/pt-write-code.result  
make tests/vm/pt-write-code2.result  
make tests/vm/pt-grow-stk-sc.result  
make tests/vm/page-linear.result  
make tests/vm/page-parallel.result  
make tests/vm/page-merge-seq.result  
make tests/vm/page-merge-par.result  
make tests/vm/page-merge-stk.result  
make tests/vm/page-merge-mm.result  
make tests/vm/page-shuffle.result  