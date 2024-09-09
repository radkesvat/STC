d=$(git rev-parse --show-toplevel)
mkdir -p $d/../stcsingle/c11
python singleheader.py $d/include/c11/fmt.h   $d/../stcsingle/c11/fmt.h
python singleheader.py $d/include/stc/algorithm.h $d/../stcsingle/stc/algorithm.h
python singleheader.py $d/include/stc/coroutine.h $d/../stcsingle/stc/coroutine.h
python singleheader.py $d/include/stc/sort.h $d/../stcsingle/stc/sort.h
python singleheader.py $d/include/stc/random.h $d/../stcsingle/stc/random.h
python singleheader.py $d/include/stc/arc.h    $d/../stcsingle/stc/arc.h
python singleheader.py $d/include/stc/cbits.h   $d/../stcsingle/stc/cbits.h
python singleheader.py $d/include/stc/box.h    $d/../stcsingle/stc/box.h
python singleheader.py $d/include/stc/common.h $d/../stcsingle/stc/common.h
python singleheader.py $d/include/stc/deque.h    $d/../stcsingle/stc/deque.h
python singleheader.py $d/include/stc/list.h   $d/../stcsingle/stc/list.h
python singleheader.py $d/include/stc/hmap.h    $d/../stcsingle/stc/hmap.h
python singleheader.py $d/include/stc/coption.h $d/../stcsingle/stc/coption.h
python singleheader.py $d/include/stc/pqueue.h   $d/../stcsingle/stc/pqueue.h
python singleheader.py $d/include/stc/queue.h  $d/../stcsingle/stc/queue.h
python singleheader.py $d/include/stc/cregex.h  $d/../stcsingle/stc/cregex.h
python singleheader.py $d/include/stc/hset.h    $d/../stcsingle/stc/hset.h
python singleheader.py $d/include/stc/smap.h   $d/../stcsingle/stc/smap.h
python singleheader.py $d/include/stc/cspan.h   $d/../stcsingle/stc/cspan.h
python singleheader.py $d/include/stc/sset.h   $d/../stcsingle/stc/sset.h
python singleheader.py $d/include/stc/stack.h  $d/../stcsingle/stc/stack.h
python singleheader.py $d/include/stc/cstr.h    $d/../stcsingle/stc/cstr.h
python singleheader.py $d/include/stc/csview.h  $d/../stcsingle/stc/csview.h
python singleheader.py $d/include/stc/zsview.h  $d/../stcsingle/stc/zsview.h
python singleheader.py $d/include/stc/vec.h    $d/../stcsingle/stc/vec.h
python singleheader.py $d/include/stc/types.h $d/../stcsingle/stc/types.h
echo "$d/../stcsingle headers updated"
